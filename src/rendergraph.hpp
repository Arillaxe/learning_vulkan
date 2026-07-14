#ifndef RENDERGRAPH_HPP
#define RENDERGRAPH_HPP

#include <vulkan/vulkan_raii.hpp>
#include <functional>
#include <utils.hpp>
#include <vk_init.hpp>

class Rendergraph
{
private:
  struct Resource
  {
    std::string name;
    vk::Format format;
    vk::Extent2D extent;
    vk::ImageUsageFlags usage;
    vk::ImageLayout initialLayout;
    vk::ImageLayout finalLayout;

    vk::raii::Image image = nullptr;
    vk::raii::DeviceMemory memory = nullptr;
    vk::raii::ImageView view = nullptr;
  };

  struct Pass
  {
    std::string name;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
    std::function<void(vk::raii::CommandBuffer &)> executeFunc;
  };

  std::unordered_map<std::string, Resource> resources;
  std::vector<Pass> passes;
  std::vector<size_t> executionOrder;

  std::vector<vk::raii::Semaphore> semaphores;
  std::vector<std::pair<size_t, size_t>> semaphoreSignalWaitPairs;

  vk::raii::Device &device;

  VkInit &init;

public:
  // TODO think how to pass device
  explicit Rendergraph(vk::raii::Device &dev, VkInit _init) : device(dev), init(_init) {}

  void addResource(
      const std::string &name,
      vk::Format format,
      vk::Extent2D extent,
      vk::ImageUsageFlags usage,
      vk::ImageLayout initialLayout,
      vk::ImageLayout finalLayout)
  {
    Resource resource;
    resource.name = name;
    resource.format = format;
    resource.extent = extent;
    resource.usage = usage;
    resource.initialLayout = initialLayout;
    resource.finalLayout = finalLayout;

    resources.emplace(name, std::move(resource));
  }

  void AddPass(
      const std::string &name,
      const std::vector<std::string> &inputs,
      const std::vector<std::string> &outputs,
      std::function<void(vk::raii::CommandBuffer &)> executeFunc)
  {
    Pass pass;
    pass.name = name;
    pass.inputs = inputs;
    pass.outputs = outputs;
    pass.executeFunc = executeFunc;

    passes.push_back(pass);
  }

  void Compile()
  {
    // Dependency Graph Construction
    // Build bidirectional dependency relationships between passes
    std::vector<std::vector<size_t>> dependencies(passes.size()); // What each pass depends on
    std::vector<std::vector<size_t>> dependents(passes.size());   // What depends on each pass

    // Track which pass produces each resource (write-after-write dependencies)
    std::unordered_map<std::string, size_t> resourceWriters;

    // Dependency Discovery Through Resource Usage Analysis
    // Analyze each pass to determine data flow relationships
    for (size_t i = 0; i < passes.size(); ++i)
    {
      const auto &pass = passes[i];

      // Process input dependencies - this pass must wait for producers
      for (const auto &input : pass.inputs)
      {
        auto it = resourceWriters.find(input);
        if (it != resourceWriters.end())
        {
          // Found the pass that produces this input - create dependency link
          dependencies[i].push_back(it->second); // This pass depends on the producer
          dependents[it->second].push_back(i);   // Producer has this as dependent
        }
      }

      // Register output production - subsequent passes may depend on these
      for (const auto &output : pass.outputs)
      {
        resourceWriters[output] = i; // Record this pass as producer
      }
    }

    // Topological Sort for Optimal Execution Order
    // Use depth-first search to compute valid execution sequence while detecting cycles
    std::vector<bool> visited(passes.size(), false); // Track completed nodes
    std::vector<bool> inStack(passes.size(), false); // Track current recursion path

    std::function<void(size_t)> visit = [&](size_t node)
    {
      if (inStack[node])
      {
        // Cycle detection - circular dependency found
        throw std::runtime_error("Cycle detected in rendergraph");
      }

      if (visited[node])
      {
        return; // Already processed this node and its dependencies
      }

      inStack[node] = true; // Mark as currently being processed

      // Recursively process all dependent passes first (post-order traversal)
      for (auto dependent : dependents[node])
      {
        visit(dependent);
      }

      inStack[node] = false;          // Remove from current path
      visited[node] = true;           // Mark as completely processed
      executionOrder.push_back(node); // Add to execution sequence
    };

    // Process all unvisited nodes to handle disconnected graph components
    for (size_t i = 0; i < passes.size(); ++i)
    {
      if (!visited[i])
      {
        visit(i);
      }
    }

    // Automatic Synchronization Object Creation
    // Generate semaphores for all dependencies identified during analysis
    for (size_t i = 0; i < passes.size(); ++i)
    {
      for (auto dep : dependencies[i])
      {
        // Create a GPU semaphore for this dependency relationship
        // The dependent pass will wait on this semaphore before executing
        semaphores.emplace_back(device.createSemaphore({}));
        semaphoreSignalWaitPairs.emplace_back(dep, i); // (producer, consumer) pair
      }
    }

    // Physical Resource Allocation and Creation
    // Transform resource descriptions into actual GPU objects
    for (auto &[name, resource] : resources)
    {
      // Configure image creation parameters based on resource description
      vk::ImageCreateInfo imageInfo;
      imageInfo.setImageType(vk::ImageType::e2D)                         // 2D texture/render target
          .setFormat(resource.format)                                    // Pixel format from description
          .setExtent({resource.extent.width, resource.extent.height, 1}) // Dimensions
          .setMipLevels(1)                                               // Single mip level for simplicity
          .setArrayLayers(1)                                             // Single layer (not array texture)
          .setSamples(vk::SampleCountFlagBits::e1)                       // No multisampling
          .setTiling(vk::ImageTiling::eOptimal)                          // GPU-optimal memory layout
          .setUsage(resource.usage)                                      // Usage flags from registration
          .setSharingMode(vk::SharingMode::eExclusive)                   // Single queue family access
          .setInitialLayout(vk::ImageLayout::eUndefined);                // Initial layout (will be transitioned)

      resource.image = device.createImage(imageInfo); // Create the GPU image object

      // Allocate backing memory for the image
      vk::MemoryRequirements memRequirements = resource.image.getMemoryRequirements();

      vk::MemoryAllocateInfo allocInfo;
      allocInfo.setAllocationSize(memRequirements.size) // Required memory size
          .setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits,
                                             vk::MemoryPropertyFlagBits::eDeviceLocal, init.physicalDevice)); // GPU-local memory

      resource.memory = device.allocateMemory(allocInfo); // Allocate GPU memory
      resource.image.bindMemory(*resource.memory, 0);     // Bind memory to image

      // Create image view for shader access
      vk::ImageViewCreateInfo viewInfo;
      viewInfo.setImage(*resource.image)                                       // Reference the created image
          .setViewType(vk::ImageViewType::e2D)                                 // 2D view type
          .setFormat(resource.format)                                          // Match image format
          .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}); // Full image access

      resource.view = device.createImageView(viewInfo); // Create shader-accessible view
    }
  }

  Resource *getResource(const std::string &name)
  {
    auto it = resources.find(name);

    return (it != resources.end()) ? &it->second : nullptr;
  }

  // Rendergraph execution engine - coordinates pass execution with automatic synchronization
  // This method transforms the compiled rendergraph into actual GPU work
  void Execute(vk::raii::CommandBuffer &commandBuffer, vk::Queue queue)
  {
    // Execution state management for dynamic synchronization
    std::vector<vk::CommandBuffer> cmdBuffers;      // Command buffer storage
    std::vector<vk::Semaphore> waitSemaphores;      // Synchronization dependencies for current pass
    std::vector<vk::PipelineStageFlags> waitStages; // Pipeline stages to wait on
    std::vector<vk::Semaphore> signalSemaphores;    // Semaphores to signal after current pass

    // Ordered Pass Execution with Automatic Dependency Management
    // Execute each pass in the computed dependency-safe order
    for (auto passIdx : executionOrder)
    {
      const auto &pass = passes[passIdx];

      // Synchronization Setup - Collect Dependencies for Current Pass
      // Determine what this pass must wait for before executing
      waitSemaphores.clear();
      waitStages.clear();

      for (size_t i = 0; i < semaphoreSignalWaitPairs.size(); ++i)
      {
        if (semaphoreSignalWaitPairs[i].second == passIdx)
        {
          // This pass depends on the completion of another pass
          waitSemaphores.push_back(*semaphores[i]);                                // Wait for dependency completion
          waitStages.push_back(vk::PipelineStageFlagBits::eColorAttachmentOutput); // Wait at output stage
        }
      }

      // Collect semaphores that this pass will signal for dependent passes
      signalSemaphores.clear();
      for (size_t i = 0; i < semaphoreSignalWaitPairs.size(); ++i)
      {
        if (semaphoreSignalWaitPairs[i].first == passIdx)
        {
          // Other passes depend on this pass's completion
          signalSemaphores.push_back(*semaphores[i]); // Signal completion for dependents
        }
      }

      // Command Buffer Preparation and Resource Layout Transitions
      // Set up command recording and transition resources to appropriate layouts
      commandBuffer.begin({}); // Begin command recording

      // Transition input resources to shader-readable layouts
      for (const auto &input : pass.inputs)
      {
        auto &resource = resources[input];

        vk::ImageMemoryBarrier barrier;
        barrier.setOldLayout(resource.initialLayout)               // Current resource layout
            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal) // Target layout for reading
            .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)       // No queue family transfer
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setImage(*resource.image)                                          // Target image
            .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}) // Full image range
            .setSrcAccessMask(vk::AccessFlagBits::eMemoryWrite)                 // Previous write access
            .setDstAccessMask(vk::AccessFlagBits::eShaderRead);                 // Required read access

        // Insert pipeline barrier for safe layout transition
        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eAllCommands,    // Wait for all previous work
            vk::PipelineStageFlagBits::eFragmentShader, // Enable fragment shader access
            vk::DependencyFlagBits::eByRegion,          // Region-local dependency
            {}, nullptr, barrier                        // Image barrier only
        );
      }

      // Transition output resources to render target layouts
      for (const auto &output : pass.outputs)
      {
        auto &resource = resources[output];

        vk::ImageMemoryBarrier barrier;
        barrier.setOldLayout(resource.initialLayout)                // Current layout state
            .setNewLayout(vk::ImageLayout::eColorAttachmentOptimal) // Optimal for color output
            .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setImage(*resource.image)
            .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1})
            .setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)            // Previous read access
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite); // Required write access

        // Insert barrier for safe transition to writable state
        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eAllCommands,
            vk::PipelineStageFlagBits::eColorAttachmentOutput, // Enable color attachment writes
            vk::DependencyFlagBits::eByRegion,
            {}, nullptr, barrier);
      }

      // Pass Execution - Execute the Actual Rendering Logic
      // Call the user-provided rendering function with prepared command buffer
      pass.executeFunc(commandBuffer); // Execute pass-specific rendering

      // Final Layout Transitions - Prepare Resources for Subsequent Use
      // Transition output resources to their final required layouts
      for (const auto &output : pass.outputs)
      {
        auto &resource = resources[output];

        vk::ImageMemoryBarrier barrier;
        barrier.setOldLayout(vk::ImageLayout::eColorAttachmentOptimal) // Current writable layout
            .setNewLayout(resource.finalLayout)                        // Required final layout
            .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setImage(*resource.image)
            .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1})
            .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite) // Previous write operations
            .setDstAccessMask(vk::AccessFlagBits::eMemoryRead);          // Enable subsequent reads

        // Insert final barrier for layout transition
        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eColorAttachmentOutput, // After color writes complete
            vk::PipelineStageFlagBits::eAllCommands,           // Before any subsequent work
            vk::DependencyFlagBits::eByRegion,
            {}, nullptr, barrier);
      }

      // Command Submission with Synchronization
      // Submit command buffer with appropriate dependency and signaling semaphores
      commandBuffer.end(); // Finalize command recording

      vk::SubmitInfo submitInfo;
      submitInfo.setWaitSemaphoreCount(static_cast<uint32_t>(waitSemaphores.size())) // Dependencies to wait for
          .setPWaitSemaphores(waitSemaphores.data())                                 // Dependency semaphores
          .setPWaitDstStageMask(waitStages.data())                                   // Pipeline stages to wait at
          .setCommandBufferCount(1)                                                  // Single command buffer
          .setPCommandBuffers(&*commandBuffer)                                       // Command buffer to execute
          .setSignalSemaphoreCount(static_cast<uint32_t>(signalSemaphores.size()))   // Semaphores to signal
          .setPSignalSemaphores(signalSemaphores.data());                            // Signal semaphores

      queue.submit(1, &submitInfo, nullptr); // Submit to GPU queue
    }
  }
};

#endif // RENDERGRAPH_HPP
