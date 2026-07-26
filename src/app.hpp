#ifndef APP_HPP
#define APP_HPP

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <core/window.hpp>
#include <renderer/renderer.hpp>
#include <core/scene.hpp>
#include <ecs/entity.hpp>
#include <ecs/components/mesh_component.hpp>
#include <ecs/components/transform_component.hpp>
#include <core/input.hpp>
#include <core/camera.hpp>
#include <core/world.hpp>
#include <core/thread_queue.hpp>
#include <thread>
#include <core/chunk_system.hpp>

class App
{
private:
  int HEIGHT = 720;
  int WIDTH = 1280;
  Window window;
  Camera camera;
  Scene scene;
  Input input;
  World world;

  std::thread chunkSystemThread;
  ThreadQueue<ChunkMesh> loadQueue;
  ThreadQueue<ChunkPos> unloadQueue;

  ChunkSystem chunkSystem;

  Renderer renderer;

  Mesh robotMesh;

public:
  App()
      : window(WIDTH, HEIGHT),
        renderer(window, scene, camera, loadQueue, unloadQueue),
        input(window, camera),
        world(123456u),
        chunkSystem(camera, world, loadQueue, unloadQueue),
        robotMesh(renderer.getVkResource(), "./models/robot.gltf")
  {
    Entity object("object");

    object.addComponent<TransformComponent>();
    object.addComponent<MeshComponent>(robotMesh);

    object.getComponent<TransformComponent>()->setPosition(glm::vec3(0.0f, 0.0f, 100.0f));
    object.getComponent<TransformComponent>()->setScale(glm::vec3(0.5f));

    // scene.addEntity(std::move(object));

    chunkSystemThread = std::thread(&ChunkSystem::run, &chunkSystem);
  }

  void run()
  {
    while (!window.shouldClose())
    {
      window.pollEvents();
      input.pollEvents();
      renderer.render();
    }

    chunkSystem.close();
    chunkSystemThread.join();

    renderer.waitIdle();
  }
};

#endif // APP_HPP
