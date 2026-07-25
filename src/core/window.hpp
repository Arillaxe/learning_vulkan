#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <renderer/vk.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Window
{
private:
  GLFWwindow *window = nullptr;
  int WIDTH;
  int HEIGHT;

  static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

public:
  bool framebufferResized = false;

  Window(int width, int height);
  ~Window();

  bool shouldClose() { return glfwWindowShouldClose(window); }
  void pollEvents() { glfwPollEvents(); }
  GLFWwindow *getGLFWWindowHandle() { return window; }
};

#endif // WINDOW_HPP
