#ifndef VK_UBO_HPP
#define VK_UBO_HPP

#include <renderer/vk/vk_context.hpp>
#include <renderer/vk/vk_resource.hpp>
#include <renderer/vk/vk_swapchain.hpp>
#include <core/camera.hpp>

class VkUbo
{
private:
  VkContext &vkContext;
  VkResource &vkResource;
  VkSwapchain &vkSwapchain;
  vk::raii::Buffer uniformBuffer;
  vk::raii::DeviceMemory uniformBufferMemory;
  void *uniformBufferMapped;

public:
  VkUbo(VkContext &context, VkResource &resource, VkSwapchain &swapchain);

  vk::raii::Buffer &getUniformBuffer() { return uniformBuffer; }
  void *getUnitformBufferMapped() { return uniformBufferMapped; }

  void updateUniformBuffer(Camera &camera);
};

#endif // VK_UBO_HPP
