#ifndef CHUNK_SYSTEM_HPP
#define CHUNK_SYSTEM_HPP

#include <core/camera.hpp>
#include <core/thread_queue.hpp>
#include <core/world.hpp>
#include <core/chunk_mesh_generator.hpp>
#include <atomic>
#include <thread>
#include <chrono>
#include <renderer/gpu_chunk_mesh.hpp>
#include <renderer/vk/vk_resource.hpp>

struct IVec2Hash
{
  std::size_t operator()(const glm::ivec2 &v) const
  {
    return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
  }
};

class ChunkSystem
{
private:
  VkResource &vkResource;
  Camera &camera;
  World &world;
  ChunkMeshGenerator chunkMeshGenerator;
  ThreadQueue<GPUChunkMesh> &loadQueue;
  ThreadQueue<glm::ivec3> &unloadQueue;
  std::atomic<bool> shouldClose = false;
  std::unordered_map<glm::ivec3, ChunkMesh> chunkMeshes;

  void update();
  std::vector<glm::ivec2> getChunksAround(glm::vec3 &position);

public:
  ChunkSystem(VkResource &resource, Camera &cam, World &w, ThreadQueue<GPUChunkMesh> &loadQueue, ThreadQueue<glm::ivec3> &unloadQueue);

  void run()
  {
    constexpr auto tickInterval = std::chrono::milliseconds(16);

    while (!shouldClose)
    {
      auto tickStart = std::chrono::steady_clock::now();

      update();

      std::this_thread::sleep_until(tickStart + tickInterval);
    }
  }

  void close()
  {
    shouldClose = true;
  }
};

#endif // CHUNK_SYSTEM_HPP
