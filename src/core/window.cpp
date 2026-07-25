#include <core/window.hpp>

void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
  auto instance = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));

  instance->framebufferResized = true;
}

Window::Window(int width, int height) : WIDTH(width), HEIGHT(height)
{
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
  glfwSetWindowUserPointer(window, this);

  glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_TRUE);
}

Window::~Window()
{
  glfwDestroyWindow(window);

  glfwTerminate();
}
