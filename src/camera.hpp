#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <GLFW/glfw3.h>

class Camera
{
private:
  // Spatial positioning and orientation vectors
  // These form the camera's local coordinate system in world space
  glm::vec3 position; // Camera's location in world coordinates
  glm::vec3 front;    // Forward direction (where camera is looking)
  glm::vec3 up;       // Camera's local up direction (for roll control)
  glm::vec3 right;    // Camera's local right direction (perpendicular to front and up)
  glm::vec3 worldUp;  // Global up vector reference (typically Y-axis)

  // Rotation representation using Euler angles
  // Provides intuitive control while managing gimbal lock and other rotation complexities
  float yaw;   // Horizontal rotation around the world up-axis (left-right looking)
  float pitch; // Vertical rotation around the camera's right axis (up-down looking)

  // User interaction and behavior parameters
  // These control how the camera responds to input and environmental factors
  float movementSpeed;    // Units per second for translation movement
  float mouseSensitivity; // Multiplier for mouse input to rotation angle conversion
  float zoom;             // Field of view control for perspective projection

  void updateCameraVectors()
  {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);

    // Recalculate the right and up vectors
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
  }

  enum class CameraMovement
  {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
  };

public:
  // Constructor with sensible defaults for common use cases
  // Provides flexibility while ensuring the camera starts in a predictable state
  Camera(
      glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), // Start at world origin
      glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),       // Y-axis as world up
      float yaw = -90.0f,                               // Look along negative Z-axis (OpenGL convention)
      float pitch = 0.0f                                // Level horizon
  )
  {
  }

  glm::mat4 getViewMatrix() const
  {
    return glm::lookAt(position, position + front, up);
  }

  glm::mat4 getProjectionMatrix(float aspectRatio, float nearPlane, float farPlane) const
  {
    return glm::perspective(glm::radians(zoom), aspectRatio, nearPlane, farPlane);
  }

  void processKeyboard(CameraMovement direction, float deltaTime)
  {
    float velocity = movementSpeed * deltaTime;

    switch (direction)
    {
    case CameraMovement::FORWARD:
      position += front * velocity;
      break;
    case CameraMovement::BACKWARD:
      position -= front * velocity;
      break;
    case CameraMovement::LEFT:
      position -= right * velocity;
      break;
    case CameraMovement::RIGHT:
      position += right * velocity;
      break;
    case CameraMovement::UP:
      position += up * velocity;
      break;
    case CameraMovement::DOWN:
      position -= up * velocity;
      break;
    }
  }

  void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true)
  {
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    yaw += xOffset;
    pitch += yOffset;

    // Constrain pitch to avoid flipping
    if (constrainPitch)
    {
      pitch = std::clamp(pitch, -89.0f, 89.0f);
    }

    // Update camera vectors based on updated Euler angles
    updateCameraVectors();
  }

  void processMouseScroll(float yOffset)
  {
  }
};

#endif // CAMERA_HPP
