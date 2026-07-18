#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <core/window.hpp>
#include <renderer/vk/vk_context.hpp>
#include <renderer/vk/vk_shader.hpp>
#include <core/scene.hpp>
#include <renderer/vk/vk_ubo.hpp>

class Renderer
{
private:
  Window &window;
  VkContext vkContext;
  Scene &scene;
  vk::raii::CommandBuffers commandBuffers;
  VkUbo globalUbo;
  vk::raii::DescriptorSetLayout descriptorSetLayout;
  vk::raii::DescriptorPool descriptorPool;
  vk::raii::DescriptorSet descriptorSet;

  vk::raii::DescriptorSetLayout createDescriptorSetLayout()
  {
    std::array<vk::DescriptorSetLayoutBinding, 1> bindings{{
        {
            .binding = 0,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eVertex,
        },
    }};

    vk::DescriptorSetLayoutCreateInfo layoutInfo{
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };

    return vk::raii::DescriptorSetLayout(vkContext.getDevice(), layoutInfo);
  }

  vk::raii::DescriptorPool createDescriptorPool()
  {
    std::array<vk::DescriptorPoolSize, 1> poolSize{{
        {
            .type = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = 1,
        },
    }};
    vk::DescriptorPoolCreateInfo poolInfo{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = 1,
        .poolSizeCount = static_cast<uint32_t>(poolSize.size()),
        .pPoolSizes = poolSize.data(),
    };

    return vk::raii::DescriptorPool(vkContext.getDevice(), poolInfo);
  }

  vk::raii::DescriptorSet createDescriptorSet()
  {
    vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &*descriptorSetLayout,
    };

    auto descriptorSet = vkContext.getDevice().allocateDescriptorSets(allocInfo);

    vk::DescriptorBufferInfo bufferInfo{
        .buffer = globalUbo.getUniformBuffer(),
        .offset = 0,
        .range = sizeof(UniformBufferObject),
    };

    std::array<vk::WriteDescriptorSet, 1> descriptorWrite{{
        {
            .dstSet = descriptorSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .pBufferInfo = &bufferInfo,
        },
    }};

    vkContext.getDevice().updateDescriptorSets(descriptorWrite, {});
  }

public:
  Renderer(Window &win, Scene &_scene)
      : window(win),
        vkContext(win),
        scene(_scene),
        commandBuffers(vkContext.getVkCommand().createCommandBuffers(1)),
        globalUbo(vkContext),
        descriptorSetLayout(createDescriptorSetLayout()),
        descriptorPool(createDescriptorPool()),
        descriptorSet(createDescriptorSet()) {}

  void render()
  {
    vkContext.getVkSynchronization().waitDrawFence();

    uint32_t imageIndex = vkContext.getVkSwapchain().acquireNextImage();
    auto &commandBuffer = commandBuffers[0];

    commandBuffer.reset();

    auto entities = scene.getEntities();

    // bind global descriptor set

    for (auto &entity : entities)
    {
      // bind local descriptor set
      // record
    }

    vkContext.getDevice().waitIdle();

    globalUbo.

        vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*vkContext.getVkSynchronization().getPresentCompleteSemaphore(),
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*vkContext.getVkSynchronization().getRenderFinishedSemaphore(),
    };

    vkContext.getQueue().submit(submitInfo, *vkContext.getVkSynchronization().getDrawFence());

    const vk::PresentInfoKHR presentInfoKHR{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*vkContext.getVkSynchronization().getRenderFinishedSemaphore(),
        .swapchainCount = 1,
        .pSwapchains = &*vkContext.getVkSwapchain().getSwapchain(),
        .pImageIndices = &imageIndex,
    };

    auto result = vkContext.getQueue().presentKHR(presentInfoKHR);

    if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR || window.framebufferResized)
    {
      window.framebufferResized = false;

      vkContext.getVkSwapchain().recreateSwapchain();
    }
    else
    {
      assert(result == vk::Result::eSuccess);
    }
  }
};

#endif // RENDERER_HPP
