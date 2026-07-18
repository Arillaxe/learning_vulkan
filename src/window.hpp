#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <GLFW/glfw3.h>
#include <camera.hpp>

class Window
{
private:
  GLFWwindow *window = nullptr;
  int WIDTH;
  int HEIGHT;

public:
  Camera &camera;
  bool framebufferResized = false;
  Window(int width, int height, Camera &cam) : WIDTH(width), HEIGHT(height), camera(cam) {}

  void init()
  {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetWindowUserPointer(window, this);

    glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_TRUE);
  }

  void setupInputCallbacks(GLFWwindow *window)
  {
    glfwSetCursorPosCallback(window, mouseCallback); // Connect mouse movement to camera rotation
    // glfwSetScrollCallback(window, scrollCallback);   // Connect scroll wheel to camera zoom

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }

  // Mouse movement callback for continuous camera rotation
  // Manages state persistence and coordinate transformations for smooth rotation control
  static void mouseCallback(GLFWwindow *window, double xpos, double ypos)
  {
    // State persistence for calculating movement deltas
    // Static variables maintain state between callback invocations
    static bool firstMouse = true;           // Flag to handle initial mouse position
    static float lastX = 0.0f, lastY = 0.0f; // Previous mouse position for delta calculation

    // Handle initial mouse position to prevent sudden camera jumps
    // First callback provides absolute position, not relative movement
    if (firstMouse)
    {
      lastX = xpos; // Initialize previous position
      lastY = ypos;
      firstMouse = false; // Disable special handling for subsequent calls
    }

    // Calculate mouse movement deltas since last callback
    // These deltas represent the amount and direction of mouse movement
    float xoffset = xpos - lastX; // Horizontal movement (left-right)
    float yoffset = lastY - ypos; // Vertical movement (inverted: screen Y increases downward, camera pitch increases upward)

    // Update state for next callback iteration
    lastX = xpos;
    lastY = ypos;

    auto instance = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    // Convert mouse movement to camera rotation
    // Delta values drive continuous camera orientation changes
    instance->camera.processMouseMovement(xoffset, yoffset);
  }

  static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
  {
    auto instance = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));

    instance->framebufferResized = true;
  }

  bool shouldClose()
  {
    return glfwWindowShouldClose(window);
  }

  void pollEvents()
  {
    glfwPollEvents();
  }

  void cleanUp()
  {
    glfwDestroyWindow(window);

    glfwTerminate();
  }

  GLFWwindow *getGLFWWindowHandle()
  {
    return window;
  }
};

#endif // WINDOW_HPP
