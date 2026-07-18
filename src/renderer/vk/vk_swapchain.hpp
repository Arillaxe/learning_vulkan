#ifndef VK_SWAPCHAIN_HPP
#define VK_SWAPCHAIN_HPP

#include <renderer/vk.hpp>
#include <core/window.hpp>
#include <renderer/vk/vk_context.hpp>
#include <algorithm>

class VkSwapchain
{
private:
  Window &window;
  VkContext &vkContext;
  vk::SurfaceCapabilitiesKHR capabilities;
  vk::Extent2D extent;
  vk::SurfaceFormatKHR surfaceFormat;
  vk::raii::SwapchainKHR swapchain;
  std::vector<vk::Image> images;
  std::vector<vk::raii::ImageView> imageViews;

  vk::SurfaceCapabilitiesKHR getCapabilities()
  {
    return vkContext
        .getPhysicalDevice()
        .getSurfaceCapabilitiesKHR(vkContext.getSurface());
  }

  vk::Extent2D getSwapExtent()
  {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
      return capabilities.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(window.getGLFWWindowHandle(), &width, &height);

    return {
        std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height),
    };
  }

  vk::SurfaceFormatKHR findSurfaceFormat()
  {
    std::vector<vk::SurfaceFormatKHR> formats =
        vkContext
            .getPhysicalDevice()
            .getSurfaceFormatsKHR(vkContext.getSurface());

    const auto formatIt = std::ranges::find_if(
        formats, [](const auto &format)
        { return format.format == vk::Format::eB8G8R8A8Srgb &&
                 format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });

    return formatIt != formats.end() ? *formatIt : formats[0];
  }

  vk::PresentModeKHR getPresentMode()
  {
    std::vector<vk::PresentModeKHR> presentModes =
        vkContext
            .getPhysicalDevice()
            .getSurfacePresentModesKHR(vkContext.getSurface());

    return std::ranges::any_of(
               presentModes, [](const auto mode)
               { return mode == vk::PresentModeKHR::eMailbox; })
               ? vk::PresentModeKHR::eMailbox
               : vk::PresentModeKHR::eFifo;
  }

  vk::raii::SwapchainKHR createSwapchain()
  {
    auto minImageCount = std::max(3u, capabilities.minImageCount);

    if (capabilities.maxImageCount > 0 && capabilities.maxImageCount < minImageCount)
    {
      minImageCount = capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR swapChainCreateInfo{
        .surface = vkContext.getSurface(),
        .minImageCount = minImageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = getPresentMode(),
        .clipped = true,
    };

    return vk::raii::SwapchainKHR(vkContext.getDevice(), swapChainCreateInfo);
  }

  std::vector<vk::raii::ImageView> createImageViews()
  {
    std::vector<vk::raii::ImageView> imageViews;

    imageViews.reserve(images.size());

    for (auto &image : images)
    {
      imageViews.emplace_back(
          vkContext.getVkResource().createImageView(
              image,
              surfaceFormat.format,
              vk::ImageAspectFlagBits::eColor,
              1));
    }

    return imageViews;
  }

public:
  VkSwapchain(VkContext &context, Window &win)
      : vkContext(context),
        window(win),
        capabilities(getCapabilities()),
        extent(getSwapExtent()),
        surfaceFormat(findSurfaceFormat()),
        swapchain(createSwapchain()),
        images(swapchain.getImages()),
        imageViews(createImageViews()) {}

  vk::SurfaceFormatKHR &getSurfaceFormat()
  {
    return surfaceFormat;
  }

  void recreateSwapchain()
  {
    int width = 0;
    int height = 0;

    glfwGetFramebufferSize(window.getGLFWWindowHandle(), &width, &height);

    while (width == 0 || height == 0)
    {
      glfwGetFramebufferSize(window.getGLFWWindowHandle(), &width, &height);
      glfwWaitEvents();
    }

    vkContext.getDevice().waitIdle();

    swapchain = createSwapchain();
    imageViews = createImageViews();
  }

  uint32_t acquireNextImage()
  {
    auto [result, imageIndex] = swapchain.acquireNextImage(UINT64_MAX, *vkContext.getVkSynchronization().getPresentCompleteSemaphore(), nullptr);

    if (result == vk::Result::eErrorOutOfDateKHR)
    {
      recreateSwapchain();

      return imageIndex;
    }

    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
    {
      assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);

      throw std::runtime_error("failed to aquire swap chain image!");
    }

    return imageIndex;
  }

  vk::raii::SwapchainKHR &getSwapchain()
  {
    return swapchain;
  }

  vk::Extent2D &getExtent()
  {
    return extent;
  }
};

#endif // VK_SWAPCHAIN_HPP
