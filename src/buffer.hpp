#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <vk.hpp>
#include <vk_init.hpp>

class Buffer
{
private:
  VkInit &vkInit;

public:
  Buffer(VkInit &init) : vkInit(init) {}

  std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
  {
    vk::BufferCreateInfo bufferInfo{
        .size = size,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive,
    };
    vk::raii::Buffer buffer = vk::raii::Buffer(vkInit.getDevice(), bufferInfo);
    vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
    vk::MemoryAllocateInfo memoryAllocateInfo{
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties),
    };
    vk::raii::DeviceMemory deviceMemory = vk::raii::DeviceMemory(vkInit.getDevice(), memoryAllocateInfo);
    buffer.bindMemory(*deviceMemory, 0);

    return {std::move(buffer), std::move(deviceMemory)};
  }

  uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
  {
    vk::PhysicalDeviceMemoryProperties memProperties = vkInit.getPhysicalDevice().getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
      if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
      {
        return i;
      }
    }

    throw std::runtime_error("failed to fuind suitable memory type!");
  }
};

#endif // BUFFER_HPP
