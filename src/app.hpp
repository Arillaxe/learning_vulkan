#ifndef APP_HPP
#define APP_HPP

#include <core/window.hpp>
#include <renderer/renderer.hpp>
#include <core/scene.hpp>
#include <ecs/entity.hpp>
#include <ecs/components/mesh_component.hpp>
#include <ecs/components/transform_component.hpp>

class App
{
private:
  int HEIGHT = 720;
  int WIDTH = 1280;
  Window window;
  Scene scene;
  Renderer renderer;

public:
  App()
      : window(WIDTH, HEIGHT),
        renderer(window, scene)
  {
    Entity object("object");

    object.addComponent<TransformComponent>();
    object.addComponent<MeshComponent>("./models/robot.gltf");

    scene.addEntity(std::move(object));
  }

  void run()
  {
    while (!window.shouldClose())
    {
      window.pollEvents();
      renderer.render();
    }
  }
};

#endif // APP_HPP
