#ifndef VK_COMMAND_HPP
#define VK_COMMAND_HPP

#include <renderer/vk/vk_context.hpp>

class VkCommand
{
private:
  VkContext &vkContext;
  vk::raii::CommandPool graphicsCommandPool;
  vk::raii::CommandPool transferCommandPool;

  vk::raii::CommandPool createCommandPool(uint32_t queueFamilyIndex);

public:
  VkCommand(VkContext &context);

  vk::raii::CommandBuffers createCommandBuffers(uint32_t amount);
  vk::raii::CommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(vk::raii::CommandBuffer &&commandBuffer);
};

#endif // VK_COMMAND_HPP
