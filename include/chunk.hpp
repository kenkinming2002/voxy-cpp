#pragma once

#include <block.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_set>

static constexpr int CHUNK_WIDTH  = 16;
static constexpr int CHUNK_HEIGHT = 256;

struct ChunkData
{
public:
  Block blocks[CHUNK_HEIGHT][CHUNK_WIDTH][CHUNK_WIDTH];

public:
  bool   mesh_invalidated_major;
  bool   mesh_invalidated_minor;
  double last_remash_time;

  std::unordered_set<glm::ivec3> pending_lighting_updates;

public:
  Block* get_block(glm::ivec3 position);
  const Block* get_block(glm::ivec3 position) const;
  void explode(glm::vec3 center, float radius);

  void major_invalidate_mesh();
  void minor_invalidate_mesh();

  void lighting_invalidate(glm::ivec3 position);
};

