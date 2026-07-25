#ifndef INPUT_HPP
#define INPUT_HPP

#include <core/window.hpp>
#include <core/camera.hpp>
#include <chrono>

class Input
{
private:
  Window &window;
  Camera &camera;

public:
  Input(Window &win, Camera &cam)
      : window(win),
        camera(cam)
  {
    camera.position = glm::vec3(0, 0, -200.0f);
    camera.rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 1, 0));
    glfwSetInputMode(window.getGLFWWindowHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }

  void pollEvents()
  {
    std::string cameraPos = std::format(
        "{:.2f} {:.2f} {:.2f}",
        camera.position.x,
        camera.position.y,
        camera.position.z);
    glfwSetWindowTitle(window.getGLFWWindowHandle(), cameraPos.c_str());

    static auto startTime = std::chrono::high_resolution_clock::now();
    static auto lastTime = startTime;

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();

    lastTime = currentTime;

    if (glfwGetKey(window.getGLFWWindowHandle(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
      glfwSetWindowShouldClose(window.getGLFWWindowHandle(), true);
    }

    if (glfwGetKey(window.getGLFWWindowHandle(), GLFW_KEY_W) == GLFW_PRESS)
    {
      glm::vec3 forward = camera.rotation * glm::vec3(0.0f, 0.0f, -1.0f);
      camera.position += forward * 100.0f * deltaTime;
    }

    if (glfwGetKey(window.getGLFWWindowHandle(), GLFW_KEY_S) == GLFW_PRESS)
    {
      glm::vec3 forward = camera.rotation * glm::vec3(0.0f, 0.0f, -1.0f);
      camera.position -= forward * 100.0f * deltaTime;
    }

    pollMouse();
  }

  void pollMouse()
  {
    static bool firstMouse = true;
    static double lastX, lastY;

    double mouseX, mouseY;
    glfwGetCursorPos(window.getGLFWWindowHandle(), &mouseX, &mouseY);

    if (firstMouse)
    {
      lastX = mouseX;
      lastY = mouseY;
      firstMouse = false;
    }

    float dx = static_cast<float>(mouseX - lastX);
    float dy = static_cast<float>(mouseY - lastY);

    lastX = mouseX;
    lastY = mouseY;

    constexpr float sensitivity = 0.002f; // radians per pixel

    // World-space yaw
    glm::quat yaw = glm::angleAxis(-dx * sensitivity, glm::vec3(0, 1, 0));

    // Camera local right vector
    glm::vec3 right = camera.rotation * glm::vec3(1, 0, 0);

    // Local-space pitch
    glm::quat pitch = glm::angleAxis(-dy * sensitivity, right);

    camera.rotation = glm::normalize(pitch * yaw * camera.rotation);
  }
};

#endif // INPUT_HPP
