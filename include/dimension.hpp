#pragma once

#include <chunk.hpp>

#include <graphics/texture_array.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <unordered_set>

#include <cstdint>

struct BlockData
{
  std::uint32_t texture_indices[6];
};

struct Dimension
{
public:
  graphics::TextureArray blocks_texture_array;
  std::vector<BlockData> block_datas;

  std::unordered_map<glm::ivec2, Chunk> chunks;
  std::unordered_set<glm::ivec3> pending_lighting_updates;

public:
  Block* get_block(glm::ivec3 position);
  const Block* get_block(glm::ivec3 position) const;

  void major_invalidate_mesh(glm::ivec3 position);
  void minor_invalidate_mesh(glm::ivec3 position);

  void lighting_invalidate(glm::ivec3 position);
};

