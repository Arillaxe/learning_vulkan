#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <renderer/vk/vk_pipeline.hpp>

class Material
{
private:
  VkContext &vkContext;
  Vk_Pipeline pipeline;

public:
  Material(VkContext &context, const std::string &shaderFilename)
      : vkContext(context),
        pipeline(context, shaderFilename) {}

  Vk_Pipeline &getPipeline()
  {
    return pipeline;
  }
};

#endif // MATERIAL_HPP
