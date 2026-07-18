#ifndef VK_COMMAND_HPP
#define VK_COMMAND_HPP

#include <renderer/vk/vk_context.hpp>

class VkCommand
{
private:
  VkContext &vkContext;
  vk::raii::CommandPool commandPool;

  vk::raii::CommandPool createCommandPool()
  {
    vk::CommandPoolCreateInfo poolInfo{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = vkContext.getQueueFamilyIndex(),
    };

    return vk::raii::CommandPool(vkContext.getDevice(), poolInfo);
  }

public:
  VkCommand(VkContext &context)
      : vkContext(context),
        commandPool(createCommandPool()) {}

  vk::raii::CommandBuffers createCommandBuffers(uint32_t amount)
  {
    vk::CommandBufferAllocateInfo allocInfo{
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = amount,
    };

    return vk::raii::CommandBuffers(vkContext.getDevice(), allocInfo);
  }

  vk::raii::CommandBuffer beginSingleTimeCommands()
  {
    vk::CommandBufferAllocateInfo allocInfo{
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };
    vk::raii::CommandBuffer commandBuffer = std::move(vk::raii::CommandBuffers(vkContext.getDevice(), allocInfo).front());
    vk::CommandBufferBeginInfo beginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    };
    commandBuffer.begin(beginInfo);

    return std::move(commandBuffer);
  }

  void endSingleTimeCommands(vk::raii::CommandBuffer &&commandBuffer)
  {
    commandBuffer.end();

    vk::SubmitInfo submitInfo{
        .commandBufferCount = 1,
        .pCommandBuffers = &*commandBuffer,
    };

    vkContext.getQueue().submit(submitInfo, nullptr);
    vkContext.getQueue().waitIdle();
  }
};

#endif // VK_COMMAND_HPP
