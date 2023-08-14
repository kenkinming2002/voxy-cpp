#include <dimension.hpp>

#include <coordinates.hpp>

Block* DimensionData::get_block(glm::ivec3 position)
{
  auto [local_position, chunk_index] = coordinates::split(position);
  auto it = chunks.find(chunk_index);
  if(it == chunks.end())
    return nullptr;

  return it->second.get_block(local_position);
}

const Block* DimensionData::get_block(glm::ivec3 position) const
{
  auto [local_position, chunk_index] = coordinates::split(position);
  auto it = chunks.find(chunk_index);
  if(it == chunks.end())
    return nullptr;

  return it->second.get_block(local_position);
}

void DimensionData::major_invalidate_mesh(glm::ivec3 position)
{
  auto [local_position, chunk_index] = coordinates::split(position);
  if(auto it = chunks.find(chunk_index); it != chunks.end())
    it->second.major_invalidate_mesh();
}

void DimensionData::minor_invalidate_mesh(glm::ivec3 position)
{
  auto [local_position, chunk_index] = coordinates::split(position);
  if(auto it = chunks.find(chunk_index); it != chunks.end())
    it->second.minor_invalidate_mesh();
}

void DimensionData::lighting_invalidate(glm::ivec3 position)
{
  auto [local_position, chunk_index] = coordinates::split(position);
  if(auto it = chunks.find(chunk_index); it != chunks.end())
    it->second.lighting_invalidate(local_position);
}

