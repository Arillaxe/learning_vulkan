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
#include <renderer/gpu_chunk_mesh.hpp>
#include <core/raycast.hpp>
#include <core/math.hpp>

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
  ThreadQueue<GPUChunkMesh> loadQueue;
  ThreadQueue<ChunkPos> unloadQueue;

  Renderer renderer;
  ChunkSystem chunkSystem;

  Mesh robotMesh;

public:
  App()
      : window(WIDTH, HEIGHT),
        renderer(window, scene, camera, loadQueue, unloadQueue),
        input(window, camera),
        world(123456u),
        chunkSystem(renderer.getVkResource(), camera, world, loadQueue, unloadQueue),
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

  void gameplay()
  {
    static bool wasPressed = false;

    if (glfwGetMouseButton(window.getGLFWWindowHandle(), GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !wasPressed)
    {
      wasPressed = true;

      glm::vec3 forward = camera.rotation * glm::vec3(0.0f, 0.0f, -1.0f);

      auto hit = raycast(world, camera.position, forward, 8.0f);

      if (hit.hit)
      {
        world.getVoxel(hit.voxel.x,
                       hit.voxel.y,
                       hit.voxel.z)
            ->type = 0;
        const int chunkX = floorDiv(hit.voxel.x, CHUNK_SIZE);
        const int chunkY = floorDiv(hit.voxel.y, CHUNK_SIZE);
        const int chunkZ = floorDiv(hit.voxel.z, CHUNK_SIZE);

        std::vector<glm::ivec3> directions = {
            {1, 0, 0},
            {-1, 0, 0},
            {0, 1, 0},
            {0, -1, 0},
            {0, 0, 1},
            {0, 0, -1},
        };

        auto *chunk = world.getChunk(chunkX, chunkY, chunkZ);

        chunk->isMeshed = false;

        for (auto &direction : directions)
        {
          auto *chunk = world.getChunk(chunkX + direction.x, chunkY + direction.y, chunkZ + direction.z);

          if (chunk)
          {
            chunk->isMeshed = false;
          }
        }
      }
    }
    else if (glfwGetMouseButton(window.getGLFWWindowHandle(), GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE)
    {
      wasPressed = false;
    }
  }

  void run()
  {
    while (!window.shouldClose())
    {
      window.pollEvents();
      input.pollEvents();
      gameplay();
      renderer.render();
    }

    chunkSystem.close();
    chunkSystemThread.join();

    renderer.waitIdle();
    // GPUChunkMeshes in the queue own VkBuffer/VkDeviceMemory; destroy them
    // before Renderer (and the VkDevice) goes away.
    loadQueue.clear();
  }
};

#endif // APP_HPP
