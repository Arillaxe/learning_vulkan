#ifndef VK_RESOURCE_HPP
#define VK_RESOURCE_HPP

#include <renderer/vk.hpp>
#include <renderer/vk/vk_context.hpp>

class VkResource
{
private:
  VkContext &vkContext;

  uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
  {
    vk::PhysicalDeviceMemoryProperties memProperties =
        vkContext
            .getPhysicalDevice()
            .getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
      if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
      {
        return i;
      }
    }

    throw std::runtime_error("failed to fuind suitable memory type!");
  }

public:
  VkResource(VkContext &context) : vkContext(context) {}

  vk::raii::ImageView createImageView(vk::Image const &image, vk::Format format, vk::ImageAspectFlagBits aspectFlags, uint32_t mipLevels) const
  {
    vk::ImageViewCreateInfo viewInfo{
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .subresourceRange = {.aspectMask = aspectFlags, .baseMipLevel = 0, .levelCount = mipLevels, .baseArrayLayer = 0, .layerCount = 1},
    };

    return vk::raii::ImageView(vkContext.getDevice(), viewInfo);
  }

  vk::raii::Buffer createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage)
  {
    vk::BufferCreateInfo bufferInfo{
        .size = size,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive,
    };

    return vk::raii::Buffer(vkContext.getDevice(), bufferInfo);
  }

  vk::raii::DeviceMemory getBufferMemory(vk::raii::Buffer &buffer, vk::MemoryPropertyFlags properties)
  {
    vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
    vk::MemoryAllocateInfo memoryAllocateInfo{
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties),
    };
    vk::raii::DeviceMemory deviceMemory = vk::raii::DeviceMemory(vkContext.getDevice(), memoryAllocateInfo);
    buffer.bindMemory(*deviceMemory, 0);

    return deviceMemory;
  }

  void copyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size)
  {
    vk::raii::CommandBuffer commandCopyBuffer = vkContext.getVkCommand().beginSingleTimeCommands();
    commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy{.size = size});
    vkContext.getVkCommand().endSingleTimeCommands(std::move(commandCopyBuffer));
  }
};

#endif // VK_RESOURCE_HPP
