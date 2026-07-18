#ifndef VK_SHADER_HPP
#define VK_SHADER_HPP

#include <renderer/vk.hpp>
#include <renderer/vk/vk_context.hpp>
#include <core/filesystem.hpp>

class VkShader
{
private:
  VkContext &vkContext;
  vk::raii::ShaderModule shaderModule;

  vk::raii::ShaderModule createShaderModule(const std::vector<char> &code) const
  {
    vk::ShaderModuleCreateInfo createInfo{
        .codeSize = code.size() * sizeof(char),
        .pCode = reinterpret_cast<const uint32_t *>(code.data()),
    };
    vk::raii::ShaderModule shaderModule(vkContext.getDevice(), createInfo);

    return shaderModule;
  }

public:
  VkShader(
      VkContext &context,
      const std::string &filename)
      : vkContext(context),
        shaderModule(createShaderModule(Filesystem::readFile(filename))) {}

  vk::raii::ShaderModule &getShaderModule()
  {
    return shaderModule;
  }
};

#endif // VK_SHADER_HPP
