#ifndef WORLD_GENERATOR_HPP
#define WORLD_GENERATOR_HPP

#include <core/voxel.hpp>
#include <renderer/vertex.hpp>
#include <core/chunk.hpp>
#include <array>
#include <vector>
#include <core/world.hpp>

constexpr int VOXEL_SIZE = 10;

extern std::array<glm::vec3, 8> offsets;
extern std::array<uint32_t, 36> voxelIndices;

std::vector<Voxel> createWorld();
std::pair<std::vector<Vertex>, std::vector<uint32_t>> chunkToVertices(World &world, Chunk &chunk, const ChunkPos &pos);

#endif // WORLD_GENERATOR_HPP
