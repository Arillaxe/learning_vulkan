#include <renderer/vk/vk_resource.hpp>

#include <array>
#include <stdexcept>

VkResource::VkResource(VkContext &context, VkCommand &command)
    : vkContext(context),
      vkCommand(command),
      descriptorPool(createDescriptorPool()) {}

uint32_t VkResource::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
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

vk::raii::DescriptorPool VkResource::createDescriptorPool()
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

vk::raii::ImageView VkResource::createImageView(vk::Image const &image, vk::Format format, vk::ImageAspectFlagBits aspectFlags, uint32_t mipLevels) const
{
  vk::ImageViewCreateInfo viewInfo{
      .image = image,
      .viewType = vk::ImageViewType::e2D,
      .format = format,
      .subresourceRange = {.aspectMask = aspectFlags, .baseMipLevel = 0, .levelCount = mipLevels, .baseArrayLayer = 0, .layerCount = 1},
  };

  return vk::raii::ImageView(vkContext.getDevice(), viewInfo);
}

vk::raii::Buffer VkResource::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage)
{
  vk::BufferCreateInfo bufferInfo{
      .size = size,
      .usage = usage,
      .sharingMode = vk::SharingMode::eExclusive,
  };

  return vk::raii::Buffer(vkContext.getDevice(), bufferInfo);
}

vk::raii::DeviceMemory VkResource::getBufferMemory(vk::raii::Buffer &buffer, vk::MemoryPropertyFlags properties)
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

vk::raii::DeviceMemory VkResource::getImageMemory(vk::raii::Image &image, vk::MemoryPropertyFlags properties)
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

void VkResource::copyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size)
{
  vk::raii::CommandBuffer commandCopyBuffer = vkCommand.beginSingleTimeCommands();
  commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy{.size = size});
  vkCommand.endSingleTimeCommands(std::move(commandCopyBuffer));
}

vk::raii::Image VkResource::createImage(
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
