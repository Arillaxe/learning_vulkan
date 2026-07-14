#ifndef VK_INIT_HPP
#define VK_INIT_HPP

#include <vulkan/vulkan_raii.hpp>

class VkInit
{
public:
  vk::raii::PhysicalDevice physicalDevice;
};

#endif // VK_INIT_HPP
