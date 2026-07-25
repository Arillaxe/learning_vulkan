#ifndef VK_SHADER_HPP
#define VK_SHADER_HPP

#include <renderer/vk.hpp>
#include <renderer/vk/vk_context.hpp>
#include <string>
#include <vector>

class VkShader
{
private:
  VkContext &vkContext;
  vk::raii::ShaderModule shaderModule;

  vk::raii::ShaderModule createShaderModule(const std::vector<char> &code) const;

public:
  VkShader(VkContext &context, const std::string &filename);

  vk::raii::ShaderModule &getShaderModule() { return shaderModule; }
};

#endif // VK_SHADER_HPP
