#ifndef SWAPCHAIN_HPP
#define SWAPCHAIN_HPP

#include <vk.hpp>
#include <vk_init.hpp>
#include <window.hpp>

class Swapchain
{
private:
  vk::Extent2D swapChainExtent;
  vk::SurfaceFormatKHR swapChainSurfaceFormat;
  vk::raii::SwapchainKHR swapChain = nullptr;
  std::vector<vk::Image> swapChainImages;
  std::vector<vk::raii::ImageView> swapChainImageViews;
  VkInit &vkInit;
  Window &window;

public:
  Swapchain(VkInit &vkinit, Window &win) : vkInit(vkinit), window(win)
  {
    init();
  }

  vk::SurfaceFormatKHR &getSwapChainSurfaceFormat()
  {
    return swapChainSurfaceFormat;
  }

  void init()
  {
    vk::SurfaceCapabilitiesKHR surfaceCapabilities = vkInit.getPhysicalDevice().getSurfaceCapabilitiesKHR(vkInit.getSurface());
    swapChainExtent = chooseSwapExtent(surfaceCapabilities);
    uint32_t minImageCount = chooseSwapMinImageCount(surfaceCapabilities);
    std::vector<vk::SurfaceFormatKHR> availableFormats = vkInit.getPhysicalDevice().getSurfaceFormatsKHR(vkInit.getSurface());
    swapChainSurfaceFormat = chooseSwapSufraceFormat(availableFormats);

    std::vector<vk::PresentModeKHR> availablePresentModes = vkInit.getPhysicalDevice().getSurfacePresentModesKHR(vkInit.getSurface());
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(availablePresentModes);

    vk::SwapchainCreateInfoKHR swapChainCreateInfo{
        .surface = vkInit.getSurface(),
        .minImageCount = minImageCount,
        .imageFormat = swapChainSurfaceFormat.format,
        .imageColorSpace = swapChainSurfaceFormat.colorSpace,
        .imageExtent = swapChainExtent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = presentMode,
        .clipped = true,
    };

    swapChain = vk::raii::SwapchainKHR(vkInit.getDevice(), swapChainCreateInfo);
    swapChainImages = swapChain.getImages();

    createImageViews();
  }

  void createImageViews()
  {
    assert(swapChainImageViews.empty());

    swapChainImageViews.reserve(swapChainImages.size());
    for (auto &image : swapChainImages)
    {
      swapChainImageViews.emplace_back(createImageView(image, swapChainSurfaceFormat.format, vk::ImageAspectFlagBits::eColor, 1));
    }
  }

  vk::raii::ImageView createImageView(vk::Image const &image, vk::Format format, vk::ImageAspectFlagBits aspectFlags, uint32_t mipLevels) const
  {
    vk::ImageViewCreateInfo viewInfo{
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .subresourceRange = {.aspectMask = aspectFlags, .baseMipLevel = 0, .levelCount = mipLevels, .baseArrayLayer = 0, .layerCount = 1},
    };

    return vk::raii::ImageView(vkInit.getDevice(), viewInfo);
  }

  vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities)
  {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
      return capabilities.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(window.getGLFWWindowHandle(), &width, &height);

    return {
        std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)};
  }

  uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &capabilities)
  {
    auto minImagCount = std::max(3u, capabilities.minImageCount);

    if (capabilities.maxImageCount > 0 && capabilities.maxImageCount < minImagCount)
    {
      minImagCount = capabilities.maxImageCount;
    }

    return minImagCount;
  }

  vk::SurfaceFormatKHR chooseSwapSufraceFormat(std::vector<vk::SurfaceFormatKHR> const &formats)
  {
    const auto formatIt = std::ranges::find_if(
        formats, [](const auto &format)
        { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });

    return formatIt != formats.end() ? *formatIt : formats[0];
  }

  vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &modes)
  {
    assert(std::ranges::any_of(
        modes,
        [](const auto mode)
        { return mode == vk::PresentModeKHR::eFifo; }));
    return std::ranges::any_of(
               modes,
               [](const auto mode)
               { return mode == vk::PresentModeKHR::eMailbox; })
               ? vk::PresentModeKHR::eMailbox
               : vk::PresentModeKHR::eFifo;
  }

  vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
  {
    for (const auto format : candidates)
    {
      vk::FormatProperties props = vkInit.getPhysicalDevice().getFormatProperties(format);

      if (
          (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) ||
          (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features))
      {
        return format;
      }
    }

    throw std::runtime_error("failed to find supported format!");
  }

  vk::Format findDepthFormat()
  {
    return findSupportedFormat(
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
  }
};

#endif // SWAPCHAIN_HPP
