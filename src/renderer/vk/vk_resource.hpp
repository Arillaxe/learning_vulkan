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
  vk::raii::DescriptorPool descriptorPool;

  uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
  vk::raii::DescriptorPool createDescriptorPool();

public:
  VkResource(VkContext &context, VkCommand &command);

  vk::raii::ImageView createImageView(vk::Image const &image, vk::Format format, vk::ImageAspectFlagBits aspectFlags, uint32_t mipLevels) const;
  vk::raii::Buffer createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage);
  vk::raii::DeviceMemory getBufferMemory(vk::raii::Buffer &buffer, vk::MemoryPropertyFlags properties);
  vk::raii::DeviceMemory getImageMemory(vk::raii::Image &image, vk::MemoryPropertyFlags properties);
  void copyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size);
  vk::raii::Image createImage(
      uint32_t width,
      uint32_t height,
      uint32_t mipLevels,
      vk::SampleCountFlagBits numSamples,
      vk::Format format,
      vk::ImageTiling tiling,
      vk::ImageUsageFlags usage);

  vk::raii::DescriptorPool &getDescriptorPool() { return descriptorPool; }
};

#endif // VK_RESOURCE_HPP
