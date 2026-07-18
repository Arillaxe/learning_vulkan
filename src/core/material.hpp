#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <renderer/vk/vk_pipeline.hpp>

class Material
{
private:
  VkContext &vkContext;
  VkPipeline pipeline;

public:
  Material(VkContext &context, const std::string &shaderFilename)
      : vkContext(context),
        pipeline(context, shaderFilename) {}

  VkPipeline &getPipeline()
  {
    return pipeline;
  }
};

#endif // MATERIAL_HPP
