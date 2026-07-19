#ifndef VK_UBO_HPP
#define VK_UBO_HPP

#include <renderer/vk/vk_context.hpp>
#include <renderer/ubo.hpp>
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <renderer/vk/vk_resource.hpp>
#include <renderer/vk/vk_swapchain.hpp>

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
  VkUbo(VkContext &context, VkResource &resource, VkSwapchain &swapchain)
      : vkContext(context),
        vkResource(resource),
        vkSwapchain(swapchain),
        uniformBuffer(vkResource.createBuffer(sizeof(UniformBufferObject), vk::BufferUsageFlagBits::eUniformBuffer)),
        uniformBufferMemory(vkResource.getBufferMemory(uniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)),
        uniformBufferMapped(uniformBufferMemory.mapMemory(0, sizeof(UniformBufferObject))) {}

  vk::raii::Buffer &getUniformBuffer()
  {
    return uniformBuffer;
  }

  void *getUnitformBufferMapped()
  {
    return uniformBufferMapped;
  }

  void updateUniformBuffer()
  {
    static auto startTime = std::chrono::high_resolution_clock::now();
    static auto lastTime = startTime;

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();

    lastTime = currentTime;

    UniformBufferObject ubo{};

    ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, -50.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.projection = glm::perspective(glm::radians(45.0f), static_cast<float>(vkSwapchain.getExtent().width) / static_cast<float>(vkSwapchain.getExtent().height), 0.1f, 100.0f);
    ubo.projection[1][1] *= -1;
    ubo.deltaTime = deltaTime * 1000;

    memcpy(uniformBufferMapped, &ubo, sizeof(ubo));
  }
};

#endif // VK_UBO_HPP
