#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>

constexpr int WIDTH = 1280;
constexpr int HEIGHT = 720;

class Renderer
{
private:
  GLFWwindow *window = nullptr;

  vk::raii::Context context;
  vk::raii::Instance instance = nullptr;
  vk::raii::PhysicalDevice physicalDevice = nullptr;
  vk::raii::Device device = nullptr;
  vk::raii::Queue graphicsQueue = nullptr;

  void initWindow()
  {
    glfwInit();

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
  }
};

#endif // RENDERER_HPP
