#ifndef DIMENSION_HPP
#define DIMENSION_HPP

#include <types/chunk.hpp>

#include <texture_array.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <unordered_set>

inline int modulo(int a, int b)
{
  int value = a % b;
  if(value < 0)
    value += b;

  assert(0 <= value);
  assert(value < b);
  return value;
}

struct Camera;
struct Dimension
{
public:
  Block* get_block(glm::ivec3 position)
  {
    // FIXME: Refactor me`
    glm::ivec3 local_position = {
      modulo(position.x, CHUNK_WIDTH),
      modulo(position.y, CHUNK_WIDTH),
      position.z
    };

    glm::ivec2 chunk_index = {
      (position.x - local_position.x) / CHUNK_WIDTH,
      (position.y - local_position.y) / CHUNK_WIDTH,
    };

    auto it = chunks.find(chunk_index);
    if(it == chunks.end())
      return nullptr;

    return it->second.get_block(local_position);
  }

  const Block* get_block(glm::ivec3 position) const
  {
    // FIXME: Refactor me`
    glm::ivec3 local_position = {
      modulo(position.x, CHUNK_WIDTH),
      modulo(position.y, CHUNK_WIDTH),
      position.z
    };

    glm::ivec2 chunk_index = {
      (position.x - local_position.x) / CHUNK_WIDTH,
      (position.y - local_position.y) / CHUNK_WIDTH,
    };

    auto it = chunks.find(chunk_index);
    if(it == chunks.end())
      return nullptr;

    return it->second.get_block(local_position);
  }

public:
  void major_invalidate_mesh(glm::ivec3 position)
  {
    glm::ivec3 local_position = {
      modulo(position.x, CHUNK_WIDTH),
      modulo(position.y, CHUNK_WIDTH),
      position.z
    };
    glm::ivec2 chunk_index = {
      (position.x - local_position.x) / CHUNK_WIDTH,
      (position.y - local_position.y) / CHUNK_WIDTH,
    };
    chunks[chunk_index].major_invalidate_mesh();
  }

  void minor_invalidate_mesh(glm::ivec3 position)
  {
    glm::ivec3 local_position = {
      modulo(position.x, CHUNK_WIDTH),
      modulo(position.y, CHUNK_WIDTH),
      position.z
    };
    glm::ivec2 chunk_index = {
      (position.x - local_position.x) / CHUNK_WIDTH,
      (position.y - local_position.y) / CHUNK_WIDTH,
    };
    chunks[chunk_index].minor_invalidate_mesh();
  }

public:
  void lighting_invalidate(glm::ivec3 position)
  {
    pending_lighting_updates.insert(position);
  }

public:
  TextureArray           blocks_texture_array;
  std::vector<BlockData> block_datas;

public:
  std::unordered_map<glm::ivec2, Chunk> chunks;
  std::unordered_set<glm::ivec3>        pending_lighting_updates;
};

#endif // DIMENSION_HPP
