#ifndef CHUNK_MESH_GENERATOR_HPP
#define CHUNK_MESH_GENERATOR_HPP

#include <core/chunk_mesh.hpp>
#include <core/chunk.hpp>
#include <core/world.hpp>

using BinaryGrid = std::array<std::array<uint32_t, CHUNK_SIZE + 2>, CHUNK_SIZE + 2>;

struct BinaryGrids
{
  BinaryGrid xy{};
  BinaryGrid xz{};
  BinaryGrid yz{};
};

class ChunkMeshGenerator
{
  World &world;

  BinaryGrids getBinaryGrids(Chunk &chunk);

public:
  ChunkMeshGenerator(World &w);

  ChunkMesh getChunkMesh(Chunk &chunk);
};

#endif // CHUNK_MESH_GENERATOR_HPP
