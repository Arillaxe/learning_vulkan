#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
#include <map>
#include <assert.h>
#include <fstream>
#include <chrono>
#include <unordered_map>
#include <random>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VK_ENABLE_BETA_EXTENSIONS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#define GLFW_INCLUDE_VULKAN // REQUIRED only for GLFW CreateWindowSurface.
#include <GLFW/glfw3.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

constexpr int PARTICLE_COUNT = 4096;

struct Particle
{
  glm::vec2 pos;
  glm::vec2 vel;
  glm::vec4 color;
};

struct Vertex
{
  glm::vec3 pos;
  glm::vec2 texCoord;

  static vk::VertexInputBindingDescription getBindingDescription()
  {
    return {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = vk::VertexInputRate::eVertex,
    };
  }

  static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescription()
  {
    return {{
        {
            .location = 0,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(Vertex, pos),
        },
        {
            .location = 1,
            .binding = 0,
            .format = vk::Format::eR32G32Sfloat,
            .offset = offsetof(Vertex, texCoord),
        },
    }};
  }

  bool operator==(const Vertex &other) const
  {
    return pos == other.pos && texCoord == other.texCoord;
  }
};

namespace std
{
  template <>
  struct hash<Vertex>
  {
    size_t operator()(Vertex const &vertex) const
    {
      return hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
    }
  };
}

struct UniformBufferObject
{
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
  float deltaTime;
};

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;
const std::string MODEL_PATH = "models/robot.gltf";
const std::string TEXTURE_PATH = "models/textures/Robot_diffuse.jpeg";

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<char const *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

static std::vector<char> readFile(const std::string &filename)
{
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open())
  {
    throw std::runtime_error("failed to open file: " + filename);
  }

  std::vector<char> buffer(file.tellg());

  file.seekg(0, std::ios::beg);
  file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

  file.close();

  return buffer;
}

class HelloTriangleApplication
{
public:
  void run()
  {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
  }

private:
  GLFWwindow *window = nullptr;

  vk::raii::Context context;
  vk::raii::Instance instance = nullptr;
  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
  vk::raii::PhysicalDevice physicalDevice = nullptr;

  std::vector<const char *> requiredDeviceExtension = {
      vk::KHRSwapchainExtensionName,
      // VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
      vk::EXTExtendedDynamicStateExtensionName,
      vk::EXTExtendedDynamicState3ExtensionName,
  };

  uint32_t queueIndex = ~0;

  vk::raii::Device device = nullptr;
  vk::raii::Queue graphicsQueue = nullptr;

  vk::raii::SurfaceKHR surface = nullptr;
  vk::Extent2D swapChainExtent;
  vk::SurfaceFormatKHR swapChainSurfaceFormat;
  vk::raii::SwapchainKHR swapChain = nullptr;
  std::vector<vk::Image> swapChainImages;
  std::vector<vk::raii::ImageView> swapChainImageViews;

  vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;

  vk::raii::PipelineLayout pipelineLayout = nullptr;
  vk::raii::Pipeline graphicsPipeline = nullptr;

  vk::raii::Pipeline computePipeline = nullptr;

  vk::raii::CommandPool commandPool = nullptr;
  std::vector<vk::raii::CommandBuffer> commandBuffers;

  std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
  std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
  std::vector<vk::raii::Fence> drawFences;

  uint32_t frameIndex = 0;

  vk::raii::Buffer vertexBuffer = nullptr;
  vk::raii::DeviceMemory vertexBufferMemory = nullptr;

  vk::raii::Buffer indexBuffer = nullptr;
  vk::raii::DeviceMemory indexBufferMemory = nullptr;

  std::vector<vk::raii::Buffer> uniformBuffers;
  std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
  std::vector<void *> uniformBuffersMapped;

  vk::raii::DescriptorPool descriptorPool = nullptr;
  std::vector<vk::raii::DescriptorSet> descriptorSets;

  vk::raii::Image textureImage = nullptr;
  vk::raii::DeviceMemory textureImageMemory = nullptr;
  vk::raii::ImageView textureImageView = nullptr;
  vk::raii::Sampler textureSampler = nullptr;

  vk::raii::Image depthImage = nullptr;
  vk::raii::DeviceMemory depthImageMemory = nullptr;
  vk::raii::ImageView depthImageView = nullptr;

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  uint32_t mipLevels = 0;

  vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;

  vk::raii::Image colorImage = nullptr;
  vk::raii::DeviceMemory colorImageMemory = nullptr;
  vk::raii::ImageView colorImageView = nullptr;

  std::vector<vk::raii::Buffer> computeStorageBuffers;
  std::vector<vk::raii::DeviceMemory> computeStorageBuffersMemory;

  bool framebufferResized = false;

  void initWindow()
  {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetWindowUserPointer(window, this);

    glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_TRUE);
  }

  void initVulkan()
  {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createDescriptorSetLayout();
    createPipelines();
    createCommandPool();
    createColorResources();
    createDepthResources();
    createTextureImage();
    createTextureImageView();
    createTextureSampler();
    loadModel();
    createComputeStorageBuffers();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();
  }

  void mainLoop()
  {
    while (!glfwWindowShouldClose(window))
    {
      glfwPollEvents();
      drawFrame();
    }

    device.waitIdle();
  }

  void cleanup()
  {
    glfwDestroyWindow(window);

    glfwTerminate();
  }

  void createInstance()
  {
    constexpr vk::ApplicationInfo appInfo{
        .pApplicationName = "Hello Triangle",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = vk::ApiVersion14,
    };

    // Get the required layers
    std::vector<char const *> requiredLayers;
    if (enableValidationLayers)
    {
      requiredLayers.assign(validationLayers.begin(), validationLayers.end());
    }

    // Check if the required layers are supported by the Vulkan implementation.
    auto layerProperties = context.enumerateInstanceLayerProperties();
    auto unsupportedLayerIt = std::ranges::find_if(requiredLayers,
                                                   [&layerProperties](auto const &requiredLayer)
                                                   {
                                                     return std::ranges::none_of(layerProperties,
                                                                                 [requiredLayer](auto const &layerProperty)
                                                                                 { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
                                                   });
    if (unsupportedLayerIt != requiredLayers.end())
    {
      throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
    }

    // Get the required extensions.
    auto requiredExtensions = getRequiredInstanceExtensions();

    // Check if the required extensions are supported by the Vulkan implementation.
    auto extensionProperties = context.enumerateInstanceExtensionProperties();
    auto unsupportedPropertyIt =
        std::ranges::find_if(requiredExtensions,
                             [&extensionProperties](auto const &requiredExtension)
                             {
                               return std::ranges::none_of(extensionProperties,
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

    instance = vk::raii::Instance(context, createInfo);
  }

  void setupDebugMessenger()
  {
    if (!enableValidationLayers)
      return;

    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
        .messageSeverity = severityFlags,
        .messageType = messageTypeFlags,
        .pfnUserCallback = &debugCallback,
    };
    debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
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

  static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *)
  {
    if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError || severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
    {
      std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
    }

    return vk::False;
  }

  bool isDeviceSuitable(vk::raii::PhysicalDevice const &physicalDevice)
  {
    // Check if the physicalDevice supports the Vulkan 1.3 API version
    bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;

    // Check if any of the queue families support graphics operations
    auto queueFamilies = physicalDevice.getQueueFamilyProperties();
    bool supportsGraphics = std::ranges::any_of(queueFamilies, [](auto const &qfp)
                                                { return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });

    // Check if all required physicalDevice extensions are available
    auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
    bool supportsAllRequiredExtensions =
        std::ranges::all_of(requiredDeviceExtension,
                            [&availableDeviceExtensions](auto const &requiredDeviceExtension)
                            {
                              return std::ranges::any_of(availableDeviceExtensions,
                                                         [requiredDeviceExtension](auto const &availableDeviceExtension)
                                                         { return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0; });
                            });

    // Check if the physicalDevice supports the required features (shader draw parameters, dynamic rendering and extended dynamic state)
    auto features = physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2,
                                                         vk::PhysicalDeviceVulkan11Features,
                                                         vk::PhysicalDeviceVulkan13Features,
                                                         vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy &&
                                    features.template get<vk::PhysicalDeviceFeatures2>().features.sampleRateShading &&
                                    features.template get<vk::PhysicalDeviceFeatures2>().features.vertexPipelineStoresAndAtomics &&
                                    features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
                                    features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
                                    features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

    auto properties = physicalDevice.template getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceExtendedDynamicState3PropertiesEXT>();

    bool supportsRequiredProperties = properties.template get<vk::PhysicalDeviceExtendedDynamicState3PropertiesEXT>().dynamicPrimitiveTopologyUnrestricted;

    // Return true if the physicalDevice meets all the criteria
    return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures && supportsRequiredProperties;
  }

  void pickPhysicalDevice()
  {
    std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

    for (const auto &device : physicalDevices)
    {
      if (isDeviceSuitable(device))
      {
        physicalDevice = device;
        msaaSamples = getMaxUsableSampleCount();
        return;
      }
    }

    throw std::runtime_error("failed to find a suitable GPU!");
  }

  void createLogicalDevice()
  {
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); ++qfpIndex)
    {
      if (
          (queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
          (queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eCompute) &&
          physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface))
      {
        queueIndex = qfpIndex;
        break;
      }
    }

    if (queueIndex == ~0)
    {
      throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
    }

    vk::StructureChain<vk::PhysicalDeviceFeatures2,
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

    device = vk::raii::Device(physicalDevice, deviceCreateInfo);
    graphicsQueue = vk::raii::Queue(device, queueIndex, 0);
  }

  void createSurface()
  {
    VkSurfaceKHR _surface;

    if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != 0)
    {
      throw std::runtime_error("failed to create window surface!");
    }

    surface = vk::raii::SurfaceKHR(instance, _surface);
  }

  vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities)
  {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
      return capabilities.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

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
    const auto formatIt = std::ranges::find_if(formats, [](const auto &format)
                                               { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });

    return formatIt != formats.end() ? *formatIt : formats[0];
  }

  vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &modes)
  {
    assert(std::ranges::any_of(modes, [](const auto mode)
                               { return mode == vk::PresentModeKHR::eFifo; }));
    return std::ranges::any_of(modes, [](const auto mode)
                               { return mode == vk::PresentModeKHR::eMailbox; })
               ? vk::PresentModeKHR::eMailbox
               : vk::PresentModeKHR::eFifo;
  }

  void createSwapChain()
  {
    vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
    swapChainExtent = chooseSwapExtent(surfaceCapabilities);
    uint32_t minImageCount = chooseSwapMinImageCount(surfaceCapabilities);
    std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(*surface);
    swapChainSurfaceFormat = chooseSwapSufraceFormat(availableFormats);

    std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR(*surface);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(availablePresentModes);

    vk::SwapchainCreateInfoKHR swapChainCreateInfo{
        .surface = *surface,
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

    swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
    swapChainImages = swapChain.getImages();
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

  [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char> &code) const
  {
    vk::ShaderModuleCreateInfo createInfo{
        .codeSize = code.size() * sizeof(char),
        .pCode = reinterpret_cast<const uint32_t *>(code.data()),
    };
    vk::raii::ShaderModule shaderModule(device, createInfo);

    return shaderModule;
  }

  void createPipelines()
  {
    auto shaderCode = readFile("slang.spv");
    vk::raii::ShaderModule shaderModule = createShaderModule(shaderCode);

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = shaderModule,
        .pName = "vertMain",
    };

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = shaderModule,
        .pName = "fragMain",
    };

    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescription();

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data(),
    };

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
        .topology = vk::PrimitiveTopology::eTriangleList,
    };

    std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor, vk::DynamicState::ePrimitiveTopology};
    vk::PipelineDynamicStateCreateInfo dynamicState{
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data(),
    };

    vk::PipelineViewportStateCreateInfo viewportState{
        .viewportCount = 1,
        .scissorCount = 1,
    };

    vk::PipelineRasterizationStateCreateInfo rasterizer{
        .depthClampEnable = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eCounterClockwise,
        .depthBiasEnable = vk::False,
        .lineWidth = 1.0f,
    };

    vk::PipelineMultisampleStateCreateInfo multisampling{
        .rasterizationSamples = msaaSamples,
        .sampleShadingEnable = vk::True,
        .minSampleShading = 0.2f,
    };

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable = vk::False,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
    };

    vk::PipelineColorBlendStateCreateInfo colorBlending{
        .logicOpEnable = vk::False,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
    };

    vk::PushConstantRange pushRange{
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .offset = 0,
        .size = sizeof(uint32_t),
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
        .setLayoutCount = 1,
        .pSetLayouts = &*descriptorSetLayout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushRange,
    };

    pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

    vk::PipelineDepthStencilStateCreateInfo depthStencil{
        .depthTestEnable = vk::True,
        .depthWriteEnable = vk::True,
        .depthCompareOp = vk::CompareOp::eLess,
        .depthBoundsTestEnable = vk::False,
        .stencilTestEnable = vk::False,
    };

    vk::StructureChain<
        vk::GraphicsPipelineCreateInfo,
        vk::PipelineRenderingCreateInfo>
        pipelineCreateInfoChain = {
            {
                .stageCount = 2,
                .pStages = shaderStages,
                .pVertexInputState = &vertexInputInfo,
                .pInputAssemblyState = &inputAssembly,
                .pViewportState = &viewportState,
                .pRasterizationState = &rasterizer,
                .pMultisampleState = &multisampling,
                .pDepthStencilState = &depthStencil,
                .pColorBlendState = &colorBlending,
                .pDynamicState = &dynamicState,
                .layout = pipelineLayout,
                .renderPass = nullptr,
            },
            {.colorAttachmentCount = 1,
             .pColorAttachmentFormats = &swapChainSurfaceFormat.format,
             .depthAttachmentFormat = findDepthFormat()},
        };

    graphicsPipeline = vk::raii::Pipeline(device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());

    vk::PipelineShaderStageCreateInfo computeShaderStageInfo{
        .stage = vk::ShaderStageFlagBits::eCompute,
        .module = shaderModule,
        .pName = "compMain",
    };

    vk::ComputePipelineCreateInfo computePipelineInfo{
        .stage = computeShaderStageInfo,
        .layout = pipelineLayout,
    };

    computePipeline = vk::raii::Pipeline(device, nullptr, computePipelineInfo);
  };

  void createCommandPool()
  {
    vk::CommandPoolCreateInfo poolInfo{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = queueIndex,
    };

    commandPool = vk::raii::CommandPool(device, poolInfo);
  }

  void createCommandBuffers()
  {
    vk::CommandBufferAllocateInfo allocInfo{
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
    };

    commandBuffers = vk::raii::CommandBuffers(device, allocInfo);
  }

  void recordCommandBuffer(uint32_t imageIndex)
  {
    commandBuffers[frameIndex].begin({});

    commandBuffers[frameIndex].bindPipeline(vk::PipelineBindPoint::eCompute, *computePipeline);
    commandBuffers[frameIndex].bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout, 0, *descriptorSets[frameIndex], nullptr);
    commandBuffers[frameIndex].dispatch(PARTICLE_COUNT / 256, 1, 1);

    vk::MemoryBarrier2 computeToGraphicsBarrier{
        .srcStageMask = vk::PipelineStageFlagBits2::eComputeShader,
        .srcAccessMask = vk::AccessFlagBits2::eShaderWrite,
        .dstStageMask = vk::PipelineStageFlagBits2::eVertexShader,
        .dstAccessMask = vk::AccessFlagBits2::eShaderRead,
    };

    vk::DependencyInfo dependencyInfo{
        .memoryBarrierCount = 1,
        .pMemoryBarriers = &computeToGraphicsBarrier,
    };

    commandBuffers[frameIndex].pipelineBarrier2(dependencyInfo);

    commandBuffers[frameIndex].pushConstants<uint32_t>(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, (uint32_t)0);

    transition_image_layout(
        swapChainImages[imageIndex],
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::ImageAspectFlagBits::eColor);

    transition_image_layout(
        *colorImage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::ImageAspectFlagBits::eColor);

    transition_image_layout(
        *depthImage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthAttachmentOptimal,
        vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
        vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
        vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
        vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
        vk::ImageAspectFlagBits::eDepth);

    vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
    vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);

    vk::RenderingAttachmentInfo attachmentInfo = {
        .imageView = colorImageView,
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .resolveMode = vk::ResolveModeFlagBits::eAverage,
        .resolveImageView = swapChainImageViews[imageIndex],
        .resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor,
    };
    vk::RenderingAttachmentInfo depthAttachmentInfo = {
        .imageView = depthImageView,
        .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .clearValue = clearDepth,
    };

    vk::RenderingInfo renderingInfo = {
        .renderArea = {.offset = {0, 0}, .extent = swapChainExtent},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo,
        .pDepthAttachment = &depthAttachmentInfo,
    };

    commandBuffers[frameIndex].beginRendering(renderingInfo);
    commandBuffers[frameIndex].bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);
    commandBuffers[frameIndex].bindVertexBuffers(0, *vertexBuffer, {0});
    commandBuffers[frameIndex].bindIndexBuffer(*indexBuffer, 0, vk::IndexTypeValue<decltype(indices)::value_type>::value);
    commandBuffers[frameIndex].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, *descriptorSets[frameIndex], nullptr);

    vk::Viewport viewport{
        0.0f,
        0.0f,
        static_cast<float>(swapChainExtent.width),
        static_cast<float>(swapChainExtent.height),
        0.0f,
        1.0f,
    };

    vk::Rect2D scissor{vk::Offset2D{0, 0}, swapChainExtent};

    commandBuffers[frameIndex].setViewport(0, viewport);
    commandBuffers[frameIndex].setScissor(0, scissor);
    commandBuffers[frameIndex].setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
    commandBuffers[frameIndex].drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    commandBuffers[frameIndex].pushConstants<uint32_t>(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, (uint32_t)1);
    commandBuffers[frameIndex].setPrimitiveTopology(vk::PrimitiveTopology::ePointList);
    commandBuffers[frameIndex].draw(PARTICLE_COUNT, 1, 0, 0);

    commandBuffers[frameIndex].endRendering();

    transition_image_layout(
        swapChainImages[imageIndex],
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe,
        vk::ImageAspectFlagBits::eColor);

    commandBuffers[frameIndex].end();
  }

  void
  transition_image_layout(
      vk::Image image,
      vk::ImageLayout old_layout,
      vk::ImageLayout new_layout,
      vk::AccessFlags2 src_access_mask,
      vk::AccessFlags2 dst_access_mask,
      vk::PipelineStageFlags2 src_stage_mask,
      vk::PipelineStageFlags2 dst_stage_mask,
      vk::ImageAspectFlags image_aspect_flags)
  {
    vk::ImageMemoryBarrier2 barrier = {
        .srcStageMask = src_stage_mask,
        .srcAccessMask = src_access_mask,
        .dstStageMask = dst_stage_mask,
        .dstAccessMask = dst_access_mask,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = image_aspect_flags,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    vk::DependencyInfo dependency_info = {
        .dependencyFlags = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier,
    };

    commandBuffers[frameIndex].pipelineBarrier2(dependency_info);
  }

  void createSyncObjects()
  {
    assert(presentCompleteSemaphores.empty() && renderFinishedSemaphores.empty() && drawFences.empty());

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      presentCompleteSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
      drawFences.emplace_back(device, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
    }

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
      renderFinishedSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
    }
  }

  void drawFrame()
  {
    auto fenceResult = device.waitForFences(*drawFences[frameIndex], vk::True, UINT64_MAX);

    if (fenceResult != vk::Result::eSuccess)
    {
      throw std::runtime_error("failed to wait for fence!");
    }

    auto [result, imageIndex] = swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphores[frameIndex], nullptr);

    if (result == vk::Result::eErrorOutOfDateKHR)
    {
      recreateSwapChain();

      return;
    }

    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
    {
      assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);

      throw std::runtime_error("failed to aquire swap chain image!");
    }

    device.resetFences(*drawFences[frameIndex]);

    commandBuffers[frameIndex].reset();
    recordCommandBuffer(imageIndex);

    graphicsQueue.waitIdle();

    updateUniformBuffer(frameIndex);

    vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*presentCompleteSemaphores[frameIndex],
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*commandBuffers[frameIndex],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*renderFinishedSemaphores[frameIndex],
    };

    graphicsQueue.submit(submitInfo, *drawFences[frameIndex]);

    const vk::PresentInfoKHR presentInfoKHR{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*renderFinishedSemaphores[frameIndex],
        .swapchainCount = 1,
        .pSwapchains = &*swapChain,
        .pImageIndices = &imageIndex,
    };

    result = graphicsQueue.presentKHR(presentInfoKHR);

    if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR || framebufferResized)
    {
      framebufferResized = false;

      recreateSwapChain();
    }
    else
    {
      assert(result == vk::Result::eSuccess);
    }

    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
  }

  static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
  {
    auto app = reinterpret_cast<HelloTriangleApplication *>(glfwGetWindowUserPointer(window));

    app->framebufferResized = true;
  }

  void cleanUpSwapChain()
  {
    swapChainImageViews.clear();
    swapChain = nullptr;
  }

  void recreateSwapChain()
  {
    int width = 0;
    int height = 0;

    glfwGetFramebufferSize(window, &width, &height);

    while (width == 0 || height == 0)
    {
      glfwGetFramebufferSize(window, &width, &height);
      glfwWaitEvents();
    }

    device.waitIdle();

    cleanUpSwapChain();
    createSwapChain();
    createImageViews();
    createColorResources();
    createDepthResources();
  }

  void createVertexBuffer()
  {
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    auto [stagingBuffer, stagingBufferMemory] = createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    void *dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);

    memcpy(dataStaging, vertices.data(), bufferSize);

    stagingBufferMemory.unmapMemory();

    std::tie(vertexBuffer, vertexBufferMemory) = createBuffer(bufferSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
  }

  uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
  {
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
      if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
      {
        return i;
      }
    }

    throw std::runtime_error("failed to fuind suitable memory type!");
  }

  std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
  {
    vk::BufferCreateInfo bufferInfo{
        .size = size,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive,
    };
    vk::raii::Buffer buffer = vk::raii::Buffer(device, bufferInfo);
    vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
    vk::MemoryAllocateInfo memoryAllocateInfo{
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties),
    };
    vk::raii::DeviceMemory deviceMemory = vk::raii::DeviceMemory(device, memoryAllocateInfo);
    buffer.bindMemory(*deviceMemory, 0);

    return {std::move(buffer), std::move(deviceMemory)};
  }

  void copyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size)
  {
    vk::raii::CommandBuffer commandCopyBuffer = beginSingleTimeCommands();
    commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy{.size = size});
    endSingleTimeCommands(std::move(commandCopyBuffer));
  }

  void createIndexBuffer()
  {
    vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    auto [stagingBuffer, stagingBufferMemory] = createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void *data = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(data, indices.data(), (size_t)bufferSize);
    stagingBufferMemory.unmapMemory();

    std::tie(indexBuffer, indexBufferMemory) = createBuffer(bufferSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);
  }

  void createDescriptorSetLayout()
  {
    std::array<vk::DescriptorSetLayoutBinding, 4> bindings{{
        {
            .binding = 0,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eCompute,
        },
        {
            .binding = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eFragment,
        },
        {
            .binding = 2,
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute,
        },
        {
            .binding = 3,
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute | vk::ShaderStageFlagBits::eVertex,
        },
    }};

    vk::DescriptorSetLayoutCreateInfo layoutInfo{
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };

    descriptorSetLayout = vk::raii::DescriptorSetLayout(device, layoutInfo);
  }

  void createUniformBuffers()
  {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
      auto [buffer, bufferMem] = createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
      uniformBuffers.emplace_back(std::move(buffer));
      uniformBuffersMemory.emplace_back(std::move(bufferMem));
      uniformBuffersMapped.emplace_back(uniformBuffersMemory.back().mapMemory(0, bufferSize));
    }
  }

  void updateUniformBuffer(uint32_t currentImage)
  {
    static auto startTime = std::chrono::high_resolution_clock::now();
    static auto lastTime = startTime;

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();

    lastTime = currentTime;

    UniformBufferObject ubo{};

    ubo.model = glm::mat4(1.0f);

    ubo.model = glm::rotate(ubo.model, time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.model = glm::scale(ubo.model, glm::vec3(0.05f));
    ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, -50.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height), 0.1f, 100.0f);
    ubo.proj[1][1] *= -1;
    ubo.deltaTime = deltaTime * 1000;

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
  }

  void createDescriptorPool()
  {
    std::array<vk::DescriptorPoolSize, 3> poolSize{{
        {
            .type = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT,
        },
        {
            .type = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT,
        },
        {
            .type = vk::DescriptorType::eStorageBuffer,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT * 2,
        },
    }};
    vk::DescriptorPoolCreateInfo poolInfo{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = MAX_FRAMES_IN_FLIGHT,
        .poolSizeCount = static_cast<uint32_t>(poolSize.size()),
        .pPoolSizes = poolSize.data(),
    };

    descriptorPool = vk::raii::DescriptorPool(device, poolInfo);
  }

  void createDescriptorSets()
  {
    std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = descriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data(),
    };

    descriptorSets = device.allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      vk::DescriptorBufferInfo bufferInfo{
          .buffer = uniformBuffers[i],
          .offset = 0,
          .range = sizeof(UniformBufferObject),
      };
      vk::DescriptorImageInfo imageInfo{
          .sampler = textureSampler,
          .imageView = textureImageView,
          .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
      };
      vk::DescriptorBufferInfo computeBufferInfoLastFrame{
          .buffer = computeStorageBuffers[(i + MAX_FRAMES_IN_FLIGHT - 1) % MAX_FRAMES_IN_FLIGHT],
          .offset = 0,
          .range = sizeof(Particle) * PARTICLE_COUNT,
      };
      vk::DescriptorBufferInfo computeBufferInfoCurrentFrame{
          .buffer = computeStorageBuffers[i],
          .offset = 0,
          .range = sizeof(Particle) * PARTICLE_COUNT,
      };
      std::array<vk::WriteDescriptorSet, 4> descriptorWrite{{
          {
              .dstSet = descriptorSets[i],
              .dstBinding = 0,
              .dstArrayElement = 0,
              .descriptorCount = 1,
              .descriptorType = vk::DescriptorType::eUniformBuffer,
              .pBufferInfo = &bufferInfo,
          },
          {
              .dstSet = descriptorSets[i],
              .dstBinding = 1,
              .dstArrayElement = 0,
              .descriptorCount = 1,
              .descriptorType = vk::DescriptorType::eCombinedImageSampler,
              .pImageInfo = &imageInfo,
          },
          {
              .dstSet = descriptorSets[i],
              .dstBinding = 2,
              .dstArrayElement = 0,
              .descriptorCount = 1,
              .descriptorType = vk::DescriptorType::eStorageBuffer,
              .pBufferInfo = &computeBufferInfoLastFrame,
          },
          {
              .dstSet = descriptorSets[i],
              .dstBinding = 3,
              .dstArrayElement = 0,
              .descriptorCount = 1,
              .descriptorType = vk::DescriptorType::eStorageBuffer,
              .pBufferInfo = &computeBufferInfoCurrentFrame,
          },
      }};

      device.updateDescriptorSets(descriptorWrite, {});
    }
  }

  void createTextureImage()
  {
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    vk::DeviceSize imageSize = texWidth * texHeight * 4;

    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (!pixels)
    {
      throw std::runtime_error("failed to load texture image!");
    }

    auto [stagingBuffer, stagingBufferMemory] = createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void *data = stagingBufferMemory.mapMemory(0, imageSize);
    memcpy(data, pixels, imageSize);
    stagingBufferMemory.unmapMemory();

    stbi_image_free(pixels);

    std::tie(textureImage, textureImageMemory) = createImage(texWidth, texHeight, mipLevels, vk::SampleCountFlagBits::e1, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal);

    vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands();
    transitionImageLayout(commandBuffer, textureImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipLevels);
    copyBufferToImage(commandBuffer, stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    // transitionImageLayout(commandBuffer, textureImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, mipLevels);
    generateMipmaps(commandBuffer, textureImage, vk::Format::eR8G8B8A8Srgb, texWidth, texHeight, mipLevels);

    endSingleTimeCommands(std::move(commandBuffer));
  }

  std::pair<vk::raii::Image, vk::raii::DeviceMemory> createImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties)
  {
    vk::ImageCreateInfo imageInfo{
        .imageType = vk::ImageType::e2D,
        .format = format,
        .extent = {width, height, 1},
        .mipLevels = mipLevels,
        .arrayLayers = 1,
        .samples = numSamples,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive,
    };

    vk::raii::Image image = vk::raii::Image(device, imageInfo);
    vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo{
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties),
    };
    vk::raii::DeviceMemory imageMemory = vk::raii::DeviceMemory(device, allocInfo);
    image.bindMemory(imageMemory, 0);

    return {std::move(image), std::move(imageMemory)};
  }

  vk::raii::CommandBuffer beginSingleTimeCommands()
  {
    vk::CommandBufferAllocateInfo allocInfo{
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };
    vk::raii::CommandBuffer commandBuffer = std::move(vk::raii::CommandBuffers(device, allocInfo).front());
    vk::CommandBufferBeginInfo beginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    };
    commandBuffer.begin(beginInfo);

    return std::move(commandBuffer);
  }

  void endSingleTimeCommands(vk::raii::CommandBuffer &&commandBuffer)
  {
    commandBuffer.end();

    vk::SubmitInfo submitInfo{
        .commandBufferCount = 1,
        .pCommandBuffers = &*commandBuffer,
    };

    graphicsQueue.submit(submitInfo, nullptr);
    graphicsQueue.waitIdle();
  }

  void transitionImageLayout(vk::raii::CommandBuffer &commandBuffer, const vk::raii::Image &image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels)
  {
    vk::ImageMemoryBarrier barrier{
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
        .image = image,
        .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor, .levelCount = mipLevels, .layerCount = 1},
    };

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
    {
      barrier.srcAccessMask = {};
      barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

      sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
      destinationStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
      barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
      barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

      sourceStage = vk::PipelineStageFlagBits::eTransfer;
      destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else
    {
      throw std::runtime_error("unsupported layout transition!");
    }

    commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr, barrier);
  }

  void copyBufferToImage(vk::raii::CommandBuffer &commandBuffer, const vk::raii::Buffer &buffer, vk::raii::Image &image, uint32_t width, uint32_t height)
  {
    vk::BufferImageCopy region{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
        .imageOffset = {0, 0, 0},
        .imageExtent = {width, height, 1},
    };

    commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
  }

  void createTextureImageView()
  {
    textureImageView = createImageView(*textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, mipLevels);
  }

  vk::raii::ImageView createImageView(vk::Image const &image, vk::Format format, vk::ImageAspectFlagBits aspectFlags, uint32_t mipLevels) const
  {
    vk::ImageViewCreateInfo viewInfo{
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .subresourceRange = {.aspectMask = aspectFlags, .baseMipLevel = 0, .levelCount = mipLevels, .baseArrayLayer = 0, .layerCount = 1},
    };

    return vk::raii::ImageView(device, viewInfo);
  }

  void createTextureSampler()
  {
    vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
    vk::SamplerCreateInfo samplerInfo{
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat,
        .anisotropyEnable = vk::True,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .compareEnable = vk::False,
        .compareOp = vk::CompareOp::eAlways,
        .minLod = 0.0f,
        .maxLod = vk::LodClampNone,
    };

    textureSampler = vk::raii::Sampler(device, samplerInfo);
  }

  void createDepthResources()
  {
    vk::Format depthFormat = findDepthFormat();
    std::tie(depthImage, depthImageMemory) = createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);
    depthImageView = createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth, 1);
  }

  vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
  {
    for (const auto format : candidates)
    {
      vk::FormatProperties props = physicalDevice.getFormatProperties(format);

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

  void loadModel()
  {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, MODEL_PATH);

    if (!warn.empty())
    {
      std::cout << "glTF warning: " << warn << std::endl;
    }

    if (!err.empty())
    {
      std::cout << "glTF error: " << err << std::endl;
    }

    if (!ret)
    {
      throw std::runtime_error("Failed to load glTF model");
    }

    vertices.clear();
    indices.clear();

    for (const auto &mesh : model.meshes)
    {
      for (const auto &primitive : mesh.primitives)
      {
        const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
        const tinygltf::BufferView &indexBufferView = model.bufferViews[indexAccessor.bufferView];
        const tinygltf::Buffer &indexBuffer = model.buffers[indexBufferView.buffer];

        const tinygltf::Accessor &posAccessor = model.accessors[primitive.attributes.at("POSITION")];
        const tinygltf::BufferView &posBufferView = model.bufferViews[posAccessor.bufferView];
        const tinygltf::Buffer &posBuffer = model.buffers[posBufferView.buffer];

        bool hasTexCoords = primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end();
        const tinygltf::Accessor *texCoordAccessor = nullptr;
        const tinygltf::BufferView *texCoordBufferView = nullptr;
        const tinygltf::Buffer *texCoordBuffer = nullptr;

        if (hasTexCoords)
        {
          texCoordAccessor = &model.accessors[primitive.attributes.at("TEXCOORD_0")];
          texCoordBufferView = &model.bufferViews[texCoordAccessor->bufferView];
          texCoordBuffer = &model.buffers[texCoordBufferView->buffer];
        }

        uint32_t baseVertex = static_cast<uint32_t>(vertices.size());

        for (size_t i = 0; i < posAccessor.count; i++)
        {
          Vertex vertex{};

          const float *pos = reinterpret_cast<const float *>(&posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset + i * 12]);
          vertex.pos = {pos[0], pos[1], pos[2]};

          if (hasTexCoords)
          {
            const float *texCoord = reinterpret_cast<const float *>(&texCoordBuffer->data[texCoordBufferView->byteOffset + texCoordAccessor->byteOffset + i * 8]);
            vertex.texCoord = {texCoord[0], texCoord[1]};
          }
          else
          {
            vertex.texCoord = {0.0f, 0.0f};
          }

          vertices.push_back(vertex);
        }

        const unsigned char *indexData = &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset];
        size_t indexCount = indexAccessor.count;

        indices.reserve(indices.size() + indexCount);

        for (size_t i = 0; i < indexCount; i++)
        {
          uint32_t index = 0;

          switch (indexAccessor.componentType)
          {
          case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            index = *reinterpret_cast<const uint16_t *>(indexData + i * sizeof(uint16_t));
            break;
          case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            index = *reinterpret_cast<const uint32_t *>(indexData + i * sizeof(uint32_t));
            break;
          case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            index = *reinterpret_cast<const uint8_t *>(indexData + i * sizeof(uint8_t));
            break;

          default:
            throw std::runtime_error("Unsupported index component type");
            break;
          }

          indices.push_back(baseVertex + index);
        }
      }
    }
  }

  void generateMipmaps(vk::raii::CommandBuffer &commandBuffer, vk::raii::Image &image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
  {
    vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(imageFormat);

    if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
    {
      throw std::runtime_error("texture image format does not support linear blitting");
    }

    vk::ImageMemoryBarrier barrier = {
        .srcAccessMask = vk::AccessFlagBits::eTransferWrite,
        .dstAccessMask = vk::AccessFlagBits::eTransferRead,
        .oldLayout = vk::ImageLayout::eTransferDstOptimal,
        .newLayout = vk::ImageLayout::eTransferSrcOptimal,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
        .image = image,
        .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor, .levelCount = 1, .layerCount = 1},
    };

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; i++)
    {
      barrier.subresourceRange.baseMipLevel = i - 1;
      barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
      barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
      barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
      barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

      commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);

      vk::ImageBlit blit = {
          .srcSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = i - 1, .layerCount = 1},
          .srcOffsets = std::array<vk::Offset3D, 2>({{}, {mipWidth, mipHeight, 1}}),
          .dstSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = i, .layerCount = 1},
          .dstOffsets = std::array<vk::Offset3D, 2>({{}, {1 < mipWidth ? mipWidth / 2 : 1, 1 < mipHeight ? mipHeight / 2 : 1, 1}}),
      };

      commandBuffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);

      barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
      barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
      barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
      barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

      commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);

      if (1 < mipWidth)
      {
        mipWidth /= 2;
      }

      if (1 < mipHeight)
      {
        mipHeight /= 2;
      }
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);
  }

  vk::SampleCountFlagBits getMaxUsableSampleCount()
  {
    vk::PhysicalDeviceProperties physicalDeviceProperties = physicalDevice.getProperties();

    vk::SampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & vk::SampleCountFlagBits::e64)
    {
      return vk::SampleCountFlagBits::e64;
    }
    if (counts & vk::SampleCountFlagBits::e32)
    {
      return vk::SampleCountFlagBits::e32;
    }
    if (counts & vk::SampleCountFlagBits::e16)
    {
      return vk::SampleCountFlagBits::e16;
    }
    if (counts & vk::SampleCountFlagBits::e8)
    {
      return vk::SampleCountFlagBits::e8;
    }
    if (counts & vk::SampleCountFlagBits::e4)
    {
      return vk::SampleCountFlagBits::e4;
    }
    if (counts & vk::SampleCountFlagBits::e2)
    {
      return vk::SampleCountFlagBits::e2;
    }

    return vk::SampleCountFlagBits::e1;
  }

  void createColorResources()
  {
    vk::Format colorFormat = swapChainSurfaceFormat.format;

    std::tie(colorImage, colorImageMemory) = createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, colorFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);
    colorImageView = createImageView(colorImage, colorFormat, vk::ImageAspectFlagBits::eColor, 1);
  }

  void createComputeStorageBuffers()
  {
    computeStorageBuffers.clear();
    computeStorageBuffersMemory.clear();

    std::default_random_engine rndEngine((unsigned)time(nullptr));
    std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);

    std::vector<Particle> particles(PARTICLE_COUNT);
    for (auto &particle : particles)
    {
      float r = 0.25f * sqrtf(rndDist(rndEngine));
      float theta = rndDist(rndEngine) * 2.0f * glm::pi<float>();
      float x = r * cosf(theta) * HEIGHT / WIDTH;
      float y = r * sinf(theta);
      particle.pos = glm::vec2(x, y);
      particle.vel = normalize(glm::vec2(x, y)) * 0.00025f;
      particle.color = glm::vec4(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine), 1.0f);
    }

    vk::DeviceSize bufferSize = sizeof(Particle) * PARTICLE_COUNT;

    auto [stagingBuffer, stagingBufferMemory] = createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void *stagingData = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(stagingData, particles.data(), bufferSize);
    stagingBufferMemory.unmapMemory();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      auto [computeStorageBufferTemp, computeStorageBufferTempMemory] = createBuffer(bufferSize, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);
      copyBuffer(stagingBuffer, computeStorageBufferTemp, bufferSize);
      computeStorageBuffers.emplace_back(std::move(computeStorageBufferTemp));
      computeStorageBuffersMemory.emplace_back(std::move(computeStorageBufferTempMemory));
    }
  }
};

int main()
{
  try
  {
    HelloTriangleApplication app;
    app.run();
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
