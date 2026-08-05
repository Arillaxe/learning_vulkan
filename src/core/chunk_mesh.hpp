#ifndef CHUNK_MESH_HPP
#define CHUNK_MESH_HPP

#include <renderer/vertex.hpp>
#include <core/chunk.hpp>

struct ChunkMesh
{
  // ChunkPos chunkPos;
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};

#endif // CHUNK_MESH_HPP
