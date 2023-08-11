#pragma once

#include <chunk.hpp>

#include <graphics/texture_array.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>

#include <cstdint>

struct BlockData
{
  std::uint32_t texture_indices[6];
};

struct Dimension
{
public:
  std::unordered_map<glm::ivec2, Chunk> chunks;

public:
  Block* get_block(glm::ivec3 position);
  const Block* get_block(glm::ivec3 position) const;

  void major_invalidate_mesh(glm::ivec3 position);
  void minor_invalidate_mesh(glm::ivec3 position);

  void lighting_invalidate(glm::ivec3 position);
};

