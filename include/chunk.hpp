#pragma once

#include <block.hpp>

#include <glm/glm.hpp>

struct Chunk
{
public:
  static constexpr int WIDTH  = 16;
  static constexpr int HEIGHT = 256;

public:
  Block blocks[Chunk::HEIGHT][Chunk::WIDTH][Chunk::WIDTH];

  bool     mesh_invalidated_major;
  bool     mesh_invalidated_minor;
  uint32_t last_remash_tick;

public:
  Block* get_block(glm::ivec3 position);
  const Block* get_block(glm::ivec3 position) const;
  void explode(glm::vec3 center, float radius);

  void major_invalidate_mesh();
  void minor_invalidate_mesh();
};

