#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <vk.hpp>
#include <render_pass_manager.hpp>
#include <culling_system.hpp>
#include <camera.hpp>
#include <geomerty_pass.hpp>
#include <lighting_pass.hpp>
#include <post_process_pass.hpp>
#include <vk_init.hpp>

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

class Renderer
{
private:
  vk::raii::Device &device;
  vk::Queue graphicsQueue;
  vk::raii::CommandPool commandPool = nullptr;

  vk::raii::CommandBuffer commandBuffer = nullptr;

  std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
  std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
  std::vector<vk::raii::Fence> drawFences;

  uint32_t frameIndex = 0;

  RenderPassManager renderPassManager;
  CullingSystem cullingSystem;

public:
  Renderer(VkInit &vkInit) : device(vkInit.getDevice()), graphicsQueue(vkInit.getGraphicsQueue())
  {
    vk::CommandPoolCreateInfo poolInfo{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = vkInit.getGraphicsQueueFamilyIndex(),
    };

    commandPool = vk::raii::CommandPool(device, poolInfo);

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

    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
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
