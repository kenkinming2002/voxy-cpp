#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <types/chunk_defs.hpp>
#include <types/block.hpp>

#include <mesh.hpp>

#include <glm/glm.hpp>

#include <memory>
#include <optional>
#include <vector>

struct ChunkData
{
  Block blocks[CHUNK_HEIGHT][CHUNK_WIDTH][CHUNK_WIDTH];
};

struct Chunk
{
public:
  std::optional<Block> get_block(glm::ivec3 position) const;
  bool set_block(glm::ivec3 position, Block block);
  void explode(glm::vec3 center, float radius);

public:
  void major_invalidate_mesh();
  void minor_invalidate_mesh();

public:
  std::unique_ptr<ChunkData> data;
  std::unique_ptr<Mesh>      mesh;

  bool     mesh_invalidated_major;
  bool     mesh_invalidated_minor;
  uint32_t last_remash_tick;
};

#endif // CHUNK_HPP
