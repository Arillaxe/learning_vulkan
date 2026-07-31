#ifndef VK_SWAPCHAIN_HPP
#define VK_SWAPCHAIN_HPP

#include <renderer/vk.hpp>
#include <core/window.hpp>
#include <renderer/vk/vk_context.hpp>
#include <renderer/vk/vk_resource.hpp>
#include <renderer/vk/vk_synchronization.hpp>
#include <vector>

class VkSwapchain
{
private:
  Window &window;
  VkContext &vkContext;
  VkResource &vkResource;
  std::vector<VkSynchronization> &vkSynchronization;
  vk::SurfaceCapabilitiesKHR capabilities;
  vk::Extent2D extent;
  vk::SurfaceFormatKHR surfaceFormat;
  vk::raii::SwapchainKHR swapchain;
  std::vector<vk::Image> images;
  std::vector<vk::raii::ImageView> imageViews;

  vk::raii::Image depthImage;
  vk::raii::DeviceMemory depthImageMemory;
  vk::raii::ImageView depthImageView;

  vk::raii::Image colorImage;
  vk::raii::DeviceMemory colorImageMemory;
  vk::raii::ImageView colorImageView;

  vk::SurfaceCapabilitiesKHR getCapabilities();
  vk::Extent2D getSwapExtent();
  vk::SurfaceFormatKHR findSurfaceFormat();
  vk::PresentModeKHR getPresentMode();
  vk::raii::SwapchainKHR createSwapchain(const vk::SwapchainKHR &oldSwapchain);
  std::vector<vk::raii::ImageView> createImageViews();
  vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

public:
  VkSwapchain(VkContext &context, Window &win, VkResource &resource, std::vector<VkSynchronization> &synchronization);

  void recreateSwapchain();
  uint32_t acquireNextImage(uint32_t frameIndex);

  vk::Format findDepthFormat();
  vk::SurfaceFormatKHR &getSurfaceFormat() { return surfaceFormat; }
  vk::raii::SwapchainKHR &getSwapchain() { return swapchain; }
  vk::Extent2D &getExtent() { return extent; }
  std::vector<vk::Image> &getImages() { return images; }
  std::vector<vk::raii::ImageView> &getImageViews() { return imageViews; }
  vk::raii::Image &getDepthImage() { return depthImage; }
  vk::raii::ImageView &getColorImageView() { return colorImageView; }
  vk::raii::Image &getColorImage() { return colorImage; }
  vk::raii::ImageView &getDepthImageView() { return depthImageView; }
};

#endif // VK_SWAPCHAIN_HPP
