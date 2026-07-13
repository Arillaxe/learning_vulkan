#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <resource.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <stb_image.h>
#include <iostream>

class Texture : public Resource
{
private:
  vk::raii::Image image = nullptr;
  vk::raii::DeviceMemory memory = nullptr;
  vk::DeviceSize offset;
  vk::raii::ImageView imageView = nullptr;
  vk::raii::Sampler sampler = nullptr;

  int width = 0;
  int height = 0;
  int channels = 0;

public:
  explicit Texture(const std::string &id) : Resource(id) {}

  ~Texture() override
  {
    unload();
  }

  bool doLoad() override
  {
    std::string filePath = "textures/" + getId() + ".png";

    unsigned char *data = loadImageData(filePath, &width, &height, &channels);
    if (!data)
    {
      return false;
    }

    createVulkanImage(data, width, height, channels);

    freeImageData(data);

    return true;
  }

  bool doUnload() override
  {
    if (isLoaded())
    {
      // TODO: unload
    }
  }

  // TODO: unsure about this
  const vk::raii::Image &getImage() const { return image; }
  const vk::raii::ImageView &getImageView() const { return imageView; }
  const vk::raii::Sampler &getSampler() const { return sampler; }

private:
  unsigned char *loadImageData(const std::string &filePath, int *width, int *height, int *channels)
  {
    stbi_uc *pixels = stbi_load(filePath.c_str(), width, height, channels, STBI_rgb_alpha);

    if (!pixels)
    {
      std::cerr << "Can't load file: " + filePath << std::endl;

      return nullptr;
    }

    return pixels;
  }

  void freeImageData(unsigned char *data)
  {
    stbi_image_free(data);
  }

  void createVulkanImage(unsigned char *data, int width, int height, int channels)
  {
    // TODO: load
  }
};

#endif // TEXTURE_HPP
