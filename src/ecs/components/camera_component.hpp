#ifndef CAMERA_COMPONENT_HPP
#define CAMERA_COMPONENT_HPP

#include <ecs/component.hpp>
#include <glm/glm.hpp>
#include <ecs/entity.hpp>
#include <ecs/components/transform_component.hpp>

class CameraComponent : public Component
{
private:
  float fieldOfView = 45.0f;
  float aspectRatio = 16.0f / 9.0f;
  float nearPlane = 0.1f;
  float farPlane = 1000.0f;

  glm::mat4 viewMatrix = glm::mat4(1.0f);
  mutable glm::mat4 projectionMatrix = glm::mat4(1.0f);
  mutable bool projectionDirty = true;

public:
  void setPerspective(float fov, float aspect, float near, float far)
  {
    fieldOfView = fov;
    aspectRatio = aspect;
    nearPlane = near;
    farPlane = far;
  }

  glm::mat4 getViewMatrix() const
  {
    auto transform = getOwner()->getComponent<TransformComponent>();
    if (transform)
    {
      glm::vec3 position = transform->getPosition();
      glm::quat rotation = transform->getRotation();

      glm::vec3 forward = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
      glm::vec3 up = rotation * glm::vec3(0.0f, 1.0f, 0.0f);

      return glm::lookAt(position, position + forward, up);
    }

    return glm::mat4(1.0f);
  }

  glm::mat4 getProjectionMatrix() const
  {
    if (projectionDirty)
    {
      projectionMatrix = glm::perspective(glm::radians(fieldOfView), aspectRatio, nearPlane, farPlane);
      projectionDirty = false;
    }

    return projectionMatrix;
  }
};

#endif // CAMERA_COMPONENT_HPP
