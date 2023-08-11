#include <dimension.hpp>

static inline int modulo(int a, int b)
{
  int value = a % b;
  if(value < 0)
    value += b;

  assert(0 <= value);
  assert(value < b);
  return value;
}

Block* Dimension::get_block(glm::ivec3 position)
{
  // FIXME: Refactor me`
  glm::ivec3 local_position = {
    modulo(position.x, Chunk::WIDTH),
    modulo(position.y, Chunk::WIDTH),
    position.z
  };

  glm::ivec2 chunk_index = {
    (position.x - local_position.x) / Chunk::WIDTH,
    (position.y - local_position.y) / Chunk::WIDTH,
  };

  auto it = chunks.find(chunk_index);
  if(it == chunks.end())
    return nullptr;

  return it->second.get_block(local_position);
}

const Block* Dimension::get_block(glm::ivec3 position) const
{
  // FIXME: Refactor me`
  glm::ivec3 local_position = {
    modulo(position.x, Chunk::WIDTH),
    modulo(position.y, Chunk::WIDTH),
    position.z
  };

  glm::ivec2 chunk_index = {
    (position.x - local_position.x) / Chunk::WIDTH,
    (position.y - local_position.y) / Chunk::WIDTH,
  };

  auto it = chunks.find(chunk_index);
  if(it == chunks.end())
    return nullptr;

  return it->second.get_block(local_position);
}

void Dimension::major_invalidate_mesh(glm::ivec3 position)
{
  glm::ivec3 local_position = {
    modulo(position.x, Chunk::WIDTH),
    modulo(position.y, Chunk::WIDTH),
    position.z
  };
  glm::ivec2 chunk_index = {
    (position.x - local_position.x) / Chunk::WIDTH,
    (position.y - local_position.y) / Chunk::WIDTH,
  };
  if(auto it = chunks.find(chunk_index); it != chunks.end())
    it->second.major_invalidate_mesh();
}

void Dimension::minor_invalidate_mesh(glm::ivec3 position)
{
  glm::ivec3 local_position = {
    modulo(position.x, Chunk::WIDTH),
    modulo(position.y, Chunk::WIDTH),
    position.z
  };
  glm::ivec2 chunk_index = {
    (position.x - local_position.x) / Chunk::WIDTH,
    (position.y - local_position.y) / Chunk::WIDTH,
  };

  if(auto it = chunks.find(chunk_index); it != chunks.end())
    it->second.minor_invalidate_mesh();
}

void Dimension::lighting_invalidate(glm::ivec3 position)
{
  glm::ivec3 local_position = {
    modulo(position.x, Chunk::WIDTH),
    modulo(position.y, Chunk::WIDTH),
    position.z
  };
  glm::ivec2 chunk_index = {
    (position.x - local_position.x) / Chunk::WIDTH,
    (position.y - local_position.y) / Chunk::WIDTH,
  };

  if(auto it = chunks.find(chunk_index); it != chunks.end())
    it->second.lighting_invalidate(local_position);
}

