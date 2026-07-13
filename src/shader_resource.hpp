#ifndef SHADER_RESOURCE_HPP
#define SHADER_RESOURCE_HPP

#include <resource.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <utils.hpp>

class Shader : public Resource
{
private:
  vk::raii::ShaderModule shaderModule = nullptr;
  vk::ShaderStageFlagBits stage;

public:
  Shader(const std::string &id, vk::ShaderStageFlagBits shaderStage) : Resource(id), stage(shaderStage) {}

  ~Shader() override
  {
    unload();
  }

  bool doLoad() override
  {
    std::string extension;

    switch (stage)
    {
    case vk::ShaderStageFlagBits::eVertex:
      extension = ".vert";
      break;
    case vk::ShaderStageFlagBits::eFragment:
      extension = ".frag";
      break;
    case vk::ShaderStageFlagBits::eCompute:
      extension = ".comp";
      break;
    default:
      return false;
    }

    std::string filePath = "shaders/" + getId() + extension + ".spv";

    std::vector<char> shaderCode;
    if (!readFile(filePath, shaderCode))
    {
      return false;
    }

    createShaderModule(shaderCode);

    return true;
  }

  bool doUnload() override
  {
    if (isLoaded())
    {
      // TODO
    }
  }

  // TODO unsure
  vk::raii::ShaderModule &getShaderModule() { return shaderModule; };
  vk::ShaderStageFlagBits getStage() { return stage; }

private:
  void createShaderModule(std::vector<char> &shaderCode)
  {
    // TODO
  }
};

#endif // SHADER_RESOURCE_HPP
