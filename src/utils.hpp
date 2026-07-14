#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <fstream>
#include <iostream>
#include <vulkan/vulkan_raii.hpp>

bool readFile(const std::string &filename, std::vector<char> &buffer)
{
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open())
  {
    std::cerr << "failed to open file: " + filename << std::endl;

    return false;
  }

  file.seekg(0, std::ios::beg);
  file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

  file.close();

  return true;
}

uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties, vk::raii::PhysicalDevice &physicalDevice)
{
  vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
  {
    if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
    {
      return i;
    }
  }

  throw std::runtime_error("failed to fuind suitable memory type!");
}

void TransitionImageLayout(vk::raii::CommandBuffer &commandBuffer,
                           vk::Image image,
                           vk::Format format,
                           vk::ImageLayout oldLayout,
                           vk::ImageLayout newLayout)
{

  // Configure the basic image memory barrier structure
  // This barrier will coordinate memory access and layout transitions
  vk::ImageMemoryBarrier barrier;
  barrier.setOldLayout(oldLayout)                                          // Current image layout state
      .setNewLayout(newLayout)                                             // Target layout after transition
      .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)                     // No queue family ownership transfer
      .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)                     // Same queue family throughout
      .setImage(image)                                                     // Target image for the transition
      .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}); // Full color image range

  // Initialize pipeline stage tracking for synchronization timing
  // These stages define when operations must complete and when new operations can begin
  vk::PipelineStageFlags sourceStage;      // When previous operations must finish
  vk::PipelineStageFlags destinationStage; // When subsequent operations can start

  // Configure synchronization for undefined-to-transfer layout transitions
  // This pattern is common when preparing images for data uploads
  if (oldLayout == vk::ImageLayout::eUndefined &&
      newLayout == vk::ImageLayout::eTransferDstOptimal)
  {

    // Configure memory access permissions for upload preparation
    barrier.setSrcAccessMask(vk::AccessFlagBits::eNone)        // No previous access to synchronize
        .setDstAccessMask(vk::AccessFlagBits::eTransferWrite); // Enable transfer write operations

    // Set pipeline stage synchronization points for upload workflow
    sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;     // No previous work to wait for
    destinationStage = vk::PipelineStageFlagBits::eTransfer; // Transfer operations can proceed

    // Configure synchronization for transfer-to-shader layout transitions
    // This pattern prepares uploaded images for shader sampling
  }
  else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
           newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
  {

    // Configure memory access transition from writing to reading
    barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite) // Previous transfer writes must complete
        .setDstAccessMask(vk::AccessFlagBits::eShaderRead);      // Enable shader read access

    // Set pipeline stage synchronization for shader usage workflow
    sourceStage = vk::PipelineStageFlagBits::eTransfer;            // Transfer operations must complete
    destinationStage = vk::PipelineStageFlagBits::eFragmentShader; // Fragment shaders can access
  }
  else
  {
    // Handle unsupported transition combinations
    // Production code would include additional common transition patterns
    throw std::invalid_argument("Unsupported layout transition!");
  }

  // Execute the pipeline barrier with configured synchronization
  // This commits the layout transition and memory synchronization to the command buffer
  commandBuffer.pipelineBarrier(
      sourceStage,                       // Wait for these operations to complete
      destinationStage,                  // Before allowing these operations to begin
      vk::DependencyFlagBits::eByRegion, // Enable region-local optimizations
      {}, nullptr, barrier               // Apply our image memory barrier
  );
}

#endif // UTILS_HPP
