#include <renderer/vk/vk_ubo.hpp>

#include <renderer/ubo.hpp>
#include <chrono>
#include <cstring>
#include <glm/gtc/matrix_transform.hpp>

VkUbo::VkUbo(VkContext &context, VkResource &resource, VkSwapchain &swapchain)
    : vkContext(context),
      vkResource(resource),
      vkSwapchain(swapchain),
      uniformBuffer(vkResource.createBuffer(sizeof(UniformBufferObject), vk::BufferUsageFlagBits::eUniformBuffer)),
      uniformBufferMemory(vkResource.getBufferMemory(uniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)),
      uniformBufferMapped(uniformBufferMemory.mapMemory(0, sizeof(UniformBufferObject))) {}

void VkUbo::updateUniformBuffer(Camera &camera)
{
  static auto startTime = std::chrono::high_resolution_clock::now();
  static auto lastTime = startTime;

  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
  float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();

  lastTime = currentTime;

  UniformBufferObject ubo{};

  ubo.view =
      glm::mat4_cast(glm::conjugate(camera.rotation)) *
      glm::translate(glm::mat4(1.0f), -camera.position);
  ubo.projection = glm::perspective(glm::radians(45.0f), static_cast<float>(vkSwapchain.getExtent().width) / static_cast<float>(vkSwapchain.getExtent().height), 0.1f, 100000.0f);
  ubo.projection[1][1] *= -1;
  ubo.deltaTime = deltaTime * 1000;

  memcpy(uniformBufferMapped, &ubo, sizeof(ubo));
}
