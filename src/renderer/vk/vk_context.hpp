#ifndef VK_CONTEXT_HPP
#define VK_CONTEXT_HPP

#include <renderer/vk.hpp>
#include <core/window.hpp>
#include <vector>

class VkContext
{
private:
  Window &window;
  vk::raii::Context context;
  vk::raii::Instance instance;
  vk::raii::DebugUtilsMessengerEXT debugMessenger;
  vk::raii::SurfaceKHR surface;
  vk::raii::PhysicalDevice physicalDevice;
  uint32_t queueIndex = ~0;
  uint32_t transferQueueIndex = ~0;
  vk::raii::Device device;
  vk::raii::Queue queue;
  vk::raii::Queue transferQueue;

  vk::raii::Instance createInstance();
  std::vector<const char *> getRequiredInstanceExtensions();
  vk::raii::DebugUtilsMessengerEXT createDebugMessenger();
  static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
      vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
      vk::DebugUtilsMessageTypeFlagsEXT type,
      const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
      void *);
  vk::raii::SurfaceKHR createSurface();
  vk::raii::PhysicalDevice pickPhysicalDevice();
  bool isDeviceSuitable(vk::raii::PhysicalDevice const &physicalDevice);
  uint32_t pickQueueFamilyIndex();
  uint32_t pickTransferQueueFamilyIndex();
  vk::raii::Device createDevice();
  vk::raii::Queue createQueue();
  vk::raii::Queue createTransferQueue();

public:
  VkContext(Window &win);

  vk::raii::SurfaceKHR &getSurface() { return surface; }
  vk::raii::PhysicalDevice &getPhysicalDevice() { return physicalDevice; }
  vk::raii::Device &getDevice() { return device; }
  uint32_t getQueueFamilyIndex() { return queueIndex; }
  uint32_t getTransferQueueFamilyIndex() { return transferQueueIndex; }
  vk::raii::Queue &getQueue() { return queue; }
  vk::raii::Queue &getTransferQueue() { return transferQueue; }
};

#endif // VK_CONTEXT_HPP
