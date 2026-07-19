#ifndef VK_RESOURCE_HPP
#define VK_RESOURCE_HPP

#include <renderer/vk.hpp>
#include <renderer/vk/vk_context.hpp>
#include <renderer/vk/vk_command.hpp>

class VkResource
{
private:
  VkContext &vkContext;
  VkCommand &vkCommand;

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
  VkResource(VkContext &context, VkCommand &command)
      : vkContext(context),
        vkCommand(command) {}

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

  vk::raii::DeviceMemory getImageMemory(vk::raii::Image &image, vk::MemoryPropertyFlags properties)
  {
    vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
    vk::MemoryAllocateInfo memoryAllocateInfo{
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties),
    };
    vk::raii::DeviceMemory deviceMemory = vk::raii::DeviceMemory(vkContext.getDevice(), memoryAllocateInfo);
    image.bindMemory(*deviceMemory, 0);

    return deviceMemory;
  }

  void copyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size)
  {
    vk::raii::CommandBuffer commandCopyBuffer = vkCommand.beginSingleTimeCommands();
    commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy{.size = size});
    vkCommand.endSingleTimeCommands(std::move(commandCopyBuffer));
  }

  vk::raii::Image createImage(
      uint32_t width,
      uint32_t height,
      uint32_t mipLevels,
      vk::SampleCountFlagBits numSamples,
      vk::Format format,
      vk::ImageTiling tiling,
      vk::ImageUsageFlags usage)
  {
    vk::ImageCreateInfo imageInfo{
        .imageType = vk::ImageType::e2D,
        .format = format,
        .extent = {width, height, 1},
        .mipLevels = mipLevels,
        .arrayLayers = 1,
        .samples = numSamples,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive,
    };

    return vk::raii::Image(vkContext.getDevice(), imageInfo);
  }
};

#endif // VK_RESOURCE_HPP
