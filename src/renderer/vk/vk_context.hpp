#ifndef VK_CONTEXT_HPP
#define VK_CONTEXT_HPP

#include <renderer/vk.hpp>
#include <core/window.hpp>
#include <ranges>
#include <iostream>

const std::vector<char const *> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

const std::vector<const char *> requiredDeviceExtension = {
    vk::KHRSwapchainExtensionName,
    vk::EXTExtendedDynamicStateExtensionName,
    vk::EXTExtendedDynamicState3ExtensionName,
};

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
  vk::raii::Device device;
  vk::raii::Queue queue;

  vk::raii::Instance createInstance()
  {
    std::runtime_error("Yo");
    constexpr vk::ApplicationInfo appInfo{
        .pApplicationName = "Hello Triangle",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = vk::ApiVersion14,
    };

    std::vector<char const *> requiredLayers;
    if (enableValidationLayers)
    {
      requiredLayers.assign(validationLayers.begin(), validationLayers.end());
    }

    auto layerProperties = context.enumerateInstanceLayerProperties();
    auto unsupportedLayerIt = std::ranges::find_if(
        requiredLayers,
        [&layerProperties](auto const &requiredLayer)
        {
          return std::ranges::none_of(
              layerProperties,
              [requiredLayer](auto const &layerProperty)
              { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
        });
    if (unsupportedLayerIt != requiredLayers.end())
    {
      throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
    }

    auto requiredExtensions = getRequiredInstanceExtensions();

    auto extensionProperties = context.enumerateInstanceExtensionProperties();
    auto unsupportedPropertyIt =
        std::ranges::find_if(
            requiredExtensions,
            [&extensionProperties](auto const &requiredExtension)
            {
              return std::ranges::none_of(
                  extensionProperties,
                  [requiredExtension](auto const &extensionProperty)
                  { return strcmp(extensionProperty.extensionName, requiredExtension) == 0; });
            });

    if (unsupportedPropertyIt != requiredExtensions.end())
    {
      throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
    }

    requiredExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

    vk::InstanceCreateInfo createInfo{
        .flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
        .ppEnabledLayerNames = requiredLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data(),
    };

    return vk::raii::Instance(context, createInfo);
  }

  std::vector<const char *> getRequiredInstanceExtensions()
  {
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (enableValidationLayers)
    {
      extensions.push_back(vk::EXTDebugUtilsExtensionName);
    }

    return extensions;
  }

  vk::raii::DebugUtilsMessengerEXT createDebugMessenger()
  {
    if (enableValidationLayers)
      return nullptr;

    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
        .messageSeverity = severityFlags,
        .messageType = messageTypeFlags,
        .pfnUserCallback = &debugCallback,
    };

    return instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
  }

  static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *)
  {
    if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError || severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
    {
      std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
    }

    return vk::False;
  }

  vk::raii::SurfaceKHR createSurface()
  {
    VkSurfaceKHR _surface;

    if (glfwCreateWindowSurface(*instance, window.getGLFWWindowHandle(), nullptr, &_surface) != 0)
    {
      throw std::runtime_error("failed to create window surface!");
    }

    return vk::raii::SurfaceKHR(instance, _surface);
  }

  vk::raii::PhysicalDevice pickPhysicalDevice()
  {
    std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

    for (const auto &device : physicalDevices)
    {
      if (isDeviceSuitable(device))
      {
        return device;
      }
    }

    throw std::runtime_error("failed to find a suitable GPU!");
  }

  bool isDeviceSuitable(vk::raii::PhysicalDevice const &physicalDevice)
  {
    bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;

    auto queueFamilies = physicalDevice.getQueueFamilyProperties();
    bool supportsGraphics = std::ranges::any_of(
        queueFamilies, [](auto const &qfp)
        { return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });

    auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
    bool supportsAllRequiredExtensions =
        std::ranges::all_of(
            requiredDeviceExtension,
            [&availableDeviceExtensions](auto const &requiredDeviceExtension)
            {
              return std::ranges::any_of(
                  availableDeviceExtensions,
                  [requiredDeviceExtension](auto const &availableDeviceExtension)
                  { return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0; });
            });

    auto features = physicalDevice.template getFeatures2<
        vk::PhysicalDeviceFeatures2,
        vk::PhysicalDeviceVulkan11Features,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    bool supportsRequiredFeatures =
        features.template get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy &&
        features.template get<vk::PhysicalDeviceFeatures2>().features.sampleRateShading &&
        features.template get<vk::PhysicalDeviceFeatures2>().features.vertexPipelineStoresAndAtomics &&
        features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
        features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
        features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;
    auto properties = physicalDevice.template getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceExtendedDynamicState3PropertiesEXT>();
    bool supportsRequiredProperties = properties.template get<vk::PhysicalDeviceExtendedDynamicState3PropertiesEXT>().dynamicPrimitiveTopologyUnrestricted;

    return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions &&
           supportsRequiredFeatures && supportsRequiredProperties;
  }

  uint32_t pickQueueFamilyIndex()
  {
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); ++qfpIndex)
    {
      if (
          queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics &&
          physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface))
      {
        return qfpIndex;
        break;
      }
    }

    throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
  }

  vk::raii::Device createDevice()
  {
    vk::StructureChain<
        vk::PhysicalDeviceFeatures2,
        vk::PhysicalDeviceVulkan11Features,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        featureChain = {
            {.features = {
                 .sampleRateShading = true,
                 .samplerAnisotropy = true,
                 .vertexPipelineStoresAndAtomics = true,
             }},
            {.shaderDrawParameters = true},
            {.synchronization2 = true, .dynamicRendering = true},
            {.extendedDynamicState = true},
        };

    float queuePriority = 0.5f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
        .queueFamilyIndex = queueIndex,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
    };

    vk::DeviceCreateInfo deviceCreateInfo{
        .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &deviceQueueCreateInfo,
        .enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size()),
        .ppEnabledExtensionNames = requiredDeviceExtension.data(),
    };

    return vk::raii::Device(physicalDevice, deviceCreateInfo);
  }

  vk::raii::Queue createQueue()
  {
    return vk::raii::Queue(device, queueIndex, 0);
  }

public:
  VkContext(Window &win)
      : window(win),
        instance(createInstance()),
        debugMessenger(createDebugMessenger()),
        surface(createSurface()),
        physicalDevice(pickPhysicalDevice()),
        queueIndex(pickQueueFamilyIndex()),
        device(createDevice()),
        queue(createQueue()) {}

  vk::raii::SurfaceKHR &getSurface()
  {
    return surface;
  }

  vk::raii::PhysicalDevice &getPhysicalDevice()
  {
    return physicalDevice;
  }

  vk::raii::Device &getDevice()
  {
    return device;
  }

  uint32_t getQueueFamilyIndex()
  {
    return queueIndex;
  }

  vk::raii::Queue &getQueue()
  {
    return queue;
  }
};

#endif // VK_CONTEXT_HPP
