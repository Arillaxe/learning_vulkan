#ifndef CHUNK_SYSTEM_HPP
#define CHUNK_SYSTEM_HPP

#include <core/camera.hpp>
#include <core/thread_queue.hpp>
#include <core/world.hpp>
#include <core/chunk_mesh_generator.hpp>
#include <atomic>

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
  Camera &camera;
  World &world;
  ChunkMeshGenerator chunkMeshGenerator;
  ThreadQueue<ChunkMesh> &loadQueue;
  ThreadQueue<ChunkPos> &unloadQueue;
  std::vector<glm::ivec2> prevChunks;
  std::atomic<bool> shouldClose = false;

  void update();
  std::vector<glm::ivec2> getChunksAround(glm::vec3 &position);
  std::vector<glm::ivec2> getChunksDiff(
      const std::vector<glm::ivec2> &oldChunks,
      const std::vector<glm::ivec2> &newChunks);

public:
  ChunkSystem(Camera &cam, World &w, ThreadQueue<ChunkMesh> &loadQueue, ThreadQueue<ChunkPos> &unloadQueue);

  void run()
  {
    while (!shouldClose)
    {
      update();
    }
  }

  void close()
  {
    shouldClose = true;
  }
};

#endif // CHUNK_SYSTEM_HPP
