#include <renderer/vk/vk_pipeline.hpp>

#include <stdexcept>

Vk_Pipeline::Vk_Pipeline(VkContext &context, VkSwapchain &swapchain, VkResource &resource, const std::string &shaderFilename)
    : vkContext(context),
      vkSwapchain(swapchain),
      vkResource(resource),
      shader(context, shaderFilename),
      vkUbo(vkContext, vkResource, vkSwapchain) {}

void Vk_Pipeline::initialize()
{
  descriptorSetLayout = createDescriptorSetLayout();
  pipelineLayout = createPipelineLayout();
  pipeline = createPipeline();
  descriptorSet = createDescriptorSet();
}

vk::VertexInputBindingDescription Vk_Pipeline::getBindingDescription()
{
  return {
      .binding = 0,
      .stride = sizeof(Vertex),
      .inputRate = vk::VertexInputRate::eVertex,
  };
}

std::array<vk::VertexInputAttributeDescription, 2> Vk_Pipeline::getAttributeDescription()
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

vk::Format Vk_Pipeline::findDepthFormat()
{
  return findSupportedFormat(
      {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
      vk::ImageTiling::eOptimal,
      vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

vk::Format Vk_Pipeline::findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
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
