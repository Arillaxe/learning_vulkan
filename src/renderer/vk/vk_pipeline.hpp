#ifndef VK_PIPELINE_HPP
#define VK_PIPELINE_HPP

#include <renderer/vk.hpp>
#include <renderer/vk/vk_context.hpp>
#include <renderer/vk/vk_shader.hpp>
#include <renderer/vertex.hpp>
#include <renderer/push.hpp>
#include <renderer/vk/vk_swapchain.hpp>

class Vk_Pipeline
{
protected:
  VkContext &vkContext;
  VkSwapchain &vkSwapchain;
  VkResource &vkResource;
  VkShader shader;
  vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
  vk::raii::PipelineLayout pipelineLayout = nullptr;
  vk::raii::Pipeline pipeline = nullptr;
  vk::raii::DescriptorSet descriptorSet = nullptr;
  VkUbo vkUbo;

  void initialize()
  {
    descriptorSetLayout = createDescriptorSetLayout();
    pipelineLayout = createPipelineLayout();
    pipeline = createPipeline();
    descriptorSet = createDescriptorSet();
  }

  virtual vk::raii::PipelineLayout createPipelineLayout() = 0;
  virtual vk::raii::Pipeline createPipeline() = 0;
  virtual vk::raii::DescriptorSetLayout createDescriptorSetLayout() = 0;
  virtual vk::raii::DescriptorSet createDescriptorSet() = 0;

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
            .offset = offsetof(Vertex, position),
        },
        {
            .location = 1,
            .binding = 0,
            .format = vk::Format::eR32G32Sfloat,
            .offset = offsetof(Vertex, texCoord),
        },
    }};
  }

  vk::Format findDepthFormat()
  {
    return findSupportedFormat(
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
  }

  vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
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

public:
  Vk_Pipeline(VkContext &context, VkSwapchain &swapchain, VkResource &resource, const std::string &shaderFilename)
      : vkContext(context),
        vkSwapchain(swapchain),
        vkResource(resource),
        shader(context, shaderFilename),
        vkUbo(vkContext, vkResource, vkSwapchain) {}

  virtual ~Vk_Pipeline() = default;

  VkUbo &getVkUbo()
  {
    return vkUbo;
  }

  vk::raii::Pipeline &getPipeline()
  {
    return pipeline;
  }

  vk::raii::PipelineLayout &getPipelineLayout()
  {
    return pipelineLayout;
  }

  vk::raii::DescriptorSet &getDescriptorSet()
  {
    return descriptorSet;
  }
};

#endif // VK_PIPELINE_HPP
