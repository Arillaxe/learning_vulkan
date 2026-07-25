#include <renderer/vk/vk_shader.hpp>

#include <core/filesystem.hpp>

VkShader::VkShader(VkContext &context, const std::string &filename)
    : vkContext(context),
      shaderModule(createShaderModule(Filesystem::readFile(filename))) {}

vk::raii::ShaderModule VkShader::createShaderModule(const std::vector<char> &code) const
{
  vk::ShaderModuleCreateInfo createInfo{
      .codeSize = code.size() * sizeof(char),
      .pCode = reinterpret_cast<const uint32_t *>(code.data()),
  };
  vk::raii::ShaderModule shaderModule(vkContext.getDevice(), createInfo);

  return shaderModule;
}
