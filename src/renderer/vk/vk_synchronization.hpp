#ifndef VK_SYNCHRONIZATION_HPP
#define VK_SYNCHRONIZATION_HPP

#include <renderer/vk/vk_context.hpp>

class VkSynchronization
{
private:
  VkContext &vkContext;
  vk::raii::Fence drawFence;
  vk::raii::Semaphore presentCompleteSemaphore;
  vk::raii::Semaphore renderFinishedSemaphore;

  vk::raii::Fence createFence()
  {
    vk::FenceCreateInfo fenceInfo{
        .flags = vk::FenceCreateFlagBits::eSignaled,
    };

    return vk::raii::Fence(vkContext.getDevice(), fenceInfo);
  }

  vk::raii::Semaphore createSemaphore()
  {
    return vk::raii::Semaphore(vkContext.getDevice(), vk::SemaphoreCreateInfo());
  }

public:
  VkSynchronization(VkContext &context)
      : vkContext(context),
        drawFence(createFence()),
        presentCompleteSemaphore(createSemaphore()),
        renderFinishedSemaphore(createSemaphore()) {}

  void waitDrawFence()
  {
    auto fenceResult = vkContext.getDevice().waitForFences(*drawFence, vk::True, UINT64_MAX);

    if (fenceResult != vk::Result::eSuccess)
    {
      throw std::runtime_error("failed to wait for fence!");
    }

    vkContext.getDevice().resetFences(*drawFence);
  }

  vk::raii::Fence &getDrawFence()
  {
    return drawFence;
  }

  vk::raii::Semaphore &getPresentCompleteSemaphore()
  {
    return presentCompleteSemaphore;
  }

  vk::raii::Semaphore &getRenderFinishedSemaphore()
  {
    return renderFinishedSemaphore;
  }
};

#endif // VK_SYNCHRONIZATION_HPP
