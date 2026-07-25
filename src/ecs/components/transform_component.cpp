#include <ecs/components/transform_component.hpp>

#include <glm/gtc/matrix_transform.hpp>

glm::mat4 TransformComponent::getTransformMatrix() const
{
  if (transformDirty)
  {
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

    transformMatrix = translationMatrix * rotationMatrix * scaleMatrix;
    transformDirty = false;
  }

  return transformMatrix;
}
