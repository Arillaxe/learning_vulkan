#ifndef TRANSFORM_COMPONENT_HPP
#define TRANSFORM_COMPONENT_HPP

#include <ecs/component.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class TransformComponent : public Component
{
private:
  glm::vec3 position = glm::vec3(0.0f);
  glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  glm::vec3 scale = glm::vec3(1.0f);

  mutable glm::mat4 transformMatrix = glm::mat4(1.0f);
  mutable bool transformDirty = true;

public:
  void setPosition(const glm::vec3 &pos)
  {
    position = pos;
    transformDirty = true;
  }

  void setRotation(const glm::quat &rot)
  {
    rotation = rot;
    transformDirty = true;
  }

  void setScale(const glm::vec3 &s)
  {
    scale = s;
    transformDirty = true;
  }

  const glm::vec3 &getPosition() const { return position; }
  const glm::quat &getRotation() const { return rotation; }
  const glm::vec3 &getScale() const { return scale; }

  glm::mat4 getTransformMatrix() const;
};

#endif // TRANSFORM_COMPONENT_HPP
