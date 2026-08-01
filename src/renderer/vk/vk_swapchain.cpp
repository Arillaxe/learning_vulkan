#include <renderer/vk/vk_swapchain.hpp>

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <cassert>

VkSwapchain::VkSwapchain(VkContext &context, Window &win, VkResource &resource, std::vector<VkSynchronization> &synchronization)
    : vkContext(context),
      vkResource(resource),
      vkSynchronization(synchronization),
      window(win),
      capabilities(getCapabilities()),
      extent(getSwapExtent()),
      surfaceFormat(findSurfaceFormat()),
      swapchain(createSwapchain(VK_NULL_HANDLE)),
      images(swapchain.getImages()),
      imageViews(createImageViews()),
      renderFinishedSemaphores(createRenderFinishedSemaphores()),
      depthImage(resource.createImage(extent.width, extent.height, 1, vk::SampleCountFlagBits::e4, findDepthFormat(), vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment)),
      depthImageMemory(resource.getImageMemory(depthImage, vk::MemoryPropertyFlagBits::eDeviceLocal)),
      depthImageView(resource.createImageView(depthImage, findDepthFormat(), vk::ImageAspectFlagBits::eDepth, 1)),
      colorImage(resource.createImage(extent.width, extent.height, 1, vk::SampleCountFlagBits::e4, surfaceFormat.format, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment)),
      colorImageMemory(resource.getImageMemory(colorImage, vk::MemoryPropertyFlagBits::eDeviceLocal)),
      colorImageView(resource.createImageView(colorImage, surfaceFormat.format, vk::ImageAspectFlagBits::eColor, 1)) {}

vk::SurfaceCapabilitiesKHR VkSwapchain::getCapabilities()
{
  return vkContext
      .getPhysicalDevice()
      .getSurfaceCapabilitiesKHR(vkContext.getSurface());
}

vk::Extent2D VkSwapchain::getSwapExtent()
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

vk::SurfaceFormatKHR VkSwapchain::findSurfaceFormat()
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

vk::PresentModeKHR VkSwapchain::getPresentMode()
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

vk::raii::SwapchainKHR VkSwapchain::createSwapchain(const vk::SwapchainKHR &oldSwapchain)
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

  swapChainCreateInfo.oldSwapchain = oldSwapchain;

  return vk::raii::SwapchainKHR(vkContext.getDevice(), swapChainCreateInfo);
}

std::vector<vk::raii::ImageView> VkSwapchain::createImageViews()
{
  std::vector<vk::raii::ImageView> imageViews;

  imageViews.reserve(images.size());

  for (auto &image : images)
  {
    imageViews.emplace_back(
        vkResource.createImageView(
            image,
            surfaceFormat.format,
            vk::ImageAspectFlagBits::eColor,
            1));
  }

  return imageViews;
}

std::vector<vk::raii::Semaphore> VkSwapchain::createRenderFinishedSemaphores()
{
  std::vector<vk::raii::Semaphore> semaphores;
  semaphores.reserve(images.size());

  for (size_t i = 0; i < images.size(); ++i)
  {
    semaphores.emplace_back(vkContext.getDevice(), vk::SemaphoreCreateInfo{});
  }

  return semaphores;
}

vk::Format VkSwapchain::findDepthFormat()
{
  return findSupportedFormat(
      {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
      vk::ImageTiling::eOptimal,
      vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

vk::Format VkSwapchain::findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
  for (const auto format : candidates)
  {
    vk::FormatProperties props = vkContext.getPhysicalDevice().getFormatProperties(format);

    if (
        (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) ||
        (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features))
    {
      return format;
    }
  }

  throw std::runtime_error("failed to find supported format!");
}

void VkSwapchain::recreateSwapchain()
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

  auto oldSwapchain = std::move(swapchain);

  capabilities = getCapabilities();
  surfaceFormat = findSurfaceFormat();
  extent = getSwapExtent();

  swapchain = createSwapchain(oldSwapchain);
  images = swapchain.getImages();
  imageViews.clear();
  imageViews = createImageViews();
  renderFinishedSemaphores.clear();
  renderFinishedSemaphores = createRenderFinishedSemaphores();

  depthImage = vkResource.createImage(extent.width, extent.height, 1, vk::SampleCountFlagBits::e4, findDepthFormat(), vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment);
  depthImageMemory = vkResource.getImageMemory(depthImage, vk::MemoryPropertyFlagBits::eDeviceLocal);
  depthImageView = vkResource.createImageView(depthImage, findDepthFormat(), vk::ImageAspectFlagBits::eDepth, 1);
  colorImage = vkResource.createImage(extent.width, extent.height, 1, vk::SampleCountFlagBits::e4, surfaceFormat.format, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment);
  colorImageMemory = vkResource.getImageMemory(colorImage, vk::MemoryPropertyFlagBits::eDeviceLocal);
  colorImageView = vkResource.createImageView(colorImage, surfaceFormat.format, vk::ImageAspectFlagBits::eColor, 1);
}

uint32_t VkSwapchain::acquireNextImage(uint32_t frameIndex)
{
  auto [result, imageIndex] = swapchain.acquireNextImage(UINT64_MAX, *vkSynchronization[frameIndex].getPresentCompleteSemaphore(), nullptr);

  if (result == vk::Result::eErrorOutOfDateKHR)
  {
    recreateSwapchain();

    return acquireNextImage(frameIndex);
  }

  if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
  {
    assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);

    throw std::runtime_error("failed to aquire swap chain image!");
  }

  return imageIndex;
}
