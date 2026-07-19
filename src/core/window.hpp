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

  static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
  {
    auto instance = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));

    instance->framebufferResized = true;
  }

public:
  bool framebufferResized = false;

  Window(int width, int height) : WIDTH(width), HEIGHT(height)
  {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetWindowUserPointer(window, this);

    glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_TRUE);
  }

  ~Window()
  {
    glfwDestroyWindow(window);

    glfwTerminate();
  }

  bool shouldClose()
  {
    return glfwWindowShouldClose(window);
  }

  void pollEvents()
  {
    glfwPollEvents();
  }

  GLFWwindow *getGLFWWindowHandle()
  {
    return window;
  }
};

#endif // WINDOW_HPP
