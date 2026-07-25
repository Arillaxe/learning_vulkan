#ifndef VK_PIPELINE_HPP
#define VK_PIPELINE_HPP

#include <renderer/vk.hpp>
#include <renderer/vk/vk_context.hpp>
#include <renderer/vk/vk_shader.hpp>
#include <renderer/vk/vk_swapchain.hpp>
#include <renderer/vk/vk_ubo.hpp>
#include <renderer/vertex.hpp>
#include <renderer/push.hpp>
#include <array>
#include <string>
#include <vector>

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

  void initialize();

  virtual vk::raii::PipelineLayout createPipelineLayout() = 0;
  virtual vk::raii::Pipeline createPipeline() = 0;
  virtual vk::raii::DescriptorSetLayout createDescriptorSetLayout() = 0;
  virtual vk::raii::DescriptorSet createDescriptorSet() = 0;

  static vk::VertexInputBindingDescription getBindingDescription();
  static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescription();

  vk::Format findDepthFormat();
  vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

public:
  Vk_Pipeline(VkContext &context, VkSwapchain &swapchain, VkResource &resource, const std::string &shaderFilename);
  virtual ~Vk_Pipeline() = default;

  VkUbo &getVkUbo() { return vkUbo; }
  vk::raii::Pipeline &getPipeline() { return pipeline; }
  vk::raii::PipelineLayout &getPipelineLayout() { return pipelineLayout; }
  vk::raii::DescriptorSet &getDescriptorSet() { return descriptorSet; }
};

#endif // VK_PIPELINE_HPP
