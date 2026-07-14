#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <vulkan/vulkan_raii.hpp>
#include <render_pass_manager.hpp>
#include <culling_system.hpp>
#include <camera.hpp>
#include <geomerty_pass.hpp>
#include <lighting_pass.hpp>
#include <post_process_pass.hpp>

class Renderer
{
private:
  vk::raii::Device &device;
  vk::Queue graphicsQueue;
  vk::raii::CommandPool commandPool = nullptr;

  RenderPassManager renderPassManager;
  CullingSystem cullingSystem;

  // Current frame resources
  vk::raii::CommandBuffer commandBuffer = nullptr;
  vk::raii::Fence fence = nullptr;
  vk::raii::Semaphore imageAvailableSemaphore = nullptr;
  vk::raii::Semaphore renderFinishedSemaphore = nullptr;

public:
  Renderer(vk::raii::Device &dev, vk::Queue queue) : device(dev), graphicsQueue(queue)
  {
    // Create command pool
    // ...

    // Create synchronization objects
    // ...

    // Set up render passes
    SetupRenderPasses();
  }

  // No need for explicit destructor with RAII objects

  void SetCamera(Camera *camera)
  {
    cullingSystem.SetCamera(camera);
  }

  void Render(const std::vector<Entity *> &entities)
  {
    // Wait for previous frame to finish
    device.waitForFences(*fence, vk::True, UINT64_MAX);
    device.resetFences(*fence);

    // Reset command buffer
    commandBuffer.reset();

    // Perform culling
    cullingSystem.CullScene(entities);

    // Record commands
    vk::CommandBufferBeginInfo beginInfo;
    commandBuffer.begin(beginInfo);

    // Execute render passes
    renderPassManager.Execute(commandBuffer);

    commandBuffer.end();

    // Submit command buffer
    vk::SubmitInfo submitInfo;

    // With vk::raii, we need to dereference the command buffer
    vk::CommandBuffer rawCommandBuffer = *commandBuffer;
    submitInfo.setCommandBufferCount(1);
    submitInfo.setPCommandBuffers(&rawCommandBuffer);

    // Set up wait and signal semaphores
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    // With vk::raii, we need to dereference the semaphores
    vk::Semaphore rawImageAvailableSemaphore = *imageAvailableSemaphore;
    vk::Semaphore rawRenderFinishedSemaphore = *renderFinishedSemaphore;

    submitInfo.setWaitSemaphoreCount(1);
    submitInfo.setPWaitSemaphores(&rawImageAvailableSemaphore);
    submitInfo.setPWaitDstStageMask(waitStages);
    submitInfo.setSignalSemaphoreCount(1);
    submitInfo.setPSignalSemaphores(&rawRenderFinishedSemaphore);

    // With vk::raii, we need to dereference the fence
    vk::Fence rawFence = *fence;
    graphicsQueue.submit(1, &submitInfo, rawFence);
  }

private:
  void SetupRenderPasses()
  {
    // Create geometry pass
    auto geometryPass = renderPassManager.AddRenderPass<GeometryPass>("GeometryPass", &cullingSystem);

    // Create lighting pass
    auto lightingPass = renderPassManager.AddRenderPass<LightingPass>("LightingPass", geometryPass);

    // Create post-process pass
    auto postProcessPass = renderPassManager.AddRenderPass<PostProcessPass>("PostProcessPass", lightingPass);

    // Add post-process effects
    // ...
  }
};

#endif // RENDERER_HPP
