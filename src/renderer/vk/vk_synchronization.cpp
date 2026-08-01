#include <renderer/vk/vk_synchronization.hpp>

#include <stdexcept>

VkSynchronization::VkSynchronization(VkContext &context)
    : vkContext(context),
      drawFence(createFence()),
      presentCompleteSemaphore(createSemaphore()) {}

vk::raii::Fence VkSynchronization::createFence()
{
  vk::FenceCreateInfo fenceInfo{
      .flags = vk::FenceCreateFlagBits::eSignaled,
  };

  return vk::raii::Fence(vkContext.getDevice(), fenceInfo);
}

vk::raii::Semaphore VkSynchronization::createSemaphore()
{
  return vk::raii::Semaphore(vkContext.getDevice(), vk::SemaphoreCreateInfo());
}

void VkSynchronization::waitDrawFence()
{
  auto fenceResult = vkContext.getDevice().waitForFences(*drawFence, vk::True, UINT64_MAX);

  if (fenceResult != vk::Result::eSuccess)
  {
    throw std::runtime_error("failed to wait for fence!");
  }

  vkContext.getDevice().resetFences(*drawFence);
}
