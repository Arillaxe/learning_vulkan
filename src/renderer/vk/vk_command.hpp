#ifndef VK_COMMAND_HPP
#define VK_COMMAND_HPP

#include <renderer/vk/vk_context.hpp>

class VkCommand
{
private:
  VkContext &vkContext;
  vk::raii::CommandPool commandPool;

  vk::raii::CommandPool createCommandPool();

public:
  VkCommand(VkContext &context);

  vk::raii::CommandBuffers createCommandBuffers(uint32_t amount);
  vk::raii::CommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(vk::raii::CommandBuffer &&commandBuffer);
};

#endif // VK_COMMAND_HPP
