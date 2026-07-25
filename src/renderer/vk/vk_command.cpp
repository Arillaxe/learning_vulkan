#include <renderer/vk/vk_command.hpp>

VkCommand::VkCommand(VkContext &context)
    : vkContext(context),
      commandPool(createCommandPool()) {}

vk::raii::CommandPool VkCommand::createCommandPool()
{
  vk::CommandPoolCreateInfo poolInfo{
      .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = vkContext.getQueueFamilyIndex(),
  };

  return vk::raii::CommandPool(vkContext.getDevice(), poolInfo);
}

vk::raii::CommandBuffers VkCommand::createCommandBuffers(uint32_t amount)
{
  vk::CommandBufferAllocateInfo allocInfo{
      .commandPool = commandPool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = amount,
  };

  return vk::raii::CommandBuffers(vkContext.getDevice(), allocInfo);
}

vk::raii::CommandBuffer VkCommand::beginSingleTimeCommands()
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

void VkCommand::endSingleTimeCommands(vk::raii::CommandBuffer &&commandBuffer)
{
  commandBuffer.end();

  vk::SubmitInfo submitInfo{
      .commandBufferCount = 1,
      .pCommandBuffers = &*commandBuffer,
  };

  vkContext.getQueue().submit(submitInfo, nullptr);
  vkContext.getQueue().waitIdle();
}
