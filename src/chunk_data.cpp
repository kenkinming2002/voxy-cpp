#include <chunk_data.hpp>
#include <chunk_info.hpp>
#include <chunk_coords.hpp>

#include <chunk_manager.hpp>

#include <glm/gtx/norm.hpp>
#include <glm/gtx/hash.hpp>

Block ChunkData::get_block(glm::ivec3 position) const
{
  if(position.x < 0 || position.x >= CHUNK_WIDTH)   return Block{ .presence = false };
  if(position.y < 0 || position.y >= CHUNK_WIDTH)   return Block{ .presence = false };
  if(position.z < 0 || position.z >= slices.size()) return Block{ .presence = false };
  return slices[position.z].blocks[position.y][position.x];
}

void ChunkData::set_block(glm::ivec3 position, Block block)
{
  if(position.x < 0 || position.x >= CHUNK_WIDTH)   return;
  if(position.y < 0 || position.y >= CHUNK_WIDTH)   return;
  if(position.z < 0 || position.z >= slices.size()) return;
  slices[position.z].blocks[position.y][position.x] = block;
}

void ChunkData::explode(glm::vec3 center, float radius)
{
  // TODO: Culling
  glm::ivec3 corner1 = glm::floor(center - glm::vec3(radius, radius, radius));
  glm::ivec3 corner2 = glm::ceil (center + glm::vec3(radius, radius, radius));
  for(int z = corner1.z; z<=corner2.z; ++z)
    for(int y = corner1.y; y<=corner2.y; ++y)
      for(int x = corner1.x; x<=corner2.x; ++x)
      {
        glm::ivec3 pos = { x, y, z };
        if(glm::length2(glm::vec3(pos) - center) < radius * radius)
          set_block(pos, Block{ .presence = false });
      }
}

ChunkData ChunkData::generate(glm::ivec2 chunk_position, const ChunkManager& chunk_manager)
{
  ChunkData chunk_data;

  std::shared_lock guard(chunk_manager.mutex());
  const ChunkInfo& chunk_info = chunk_manager.chunk_infos().at(chunk_position);

  // 1: Create terrain based on height maps
  int max_height = 0;
  for(int ly=0; ly<CHUNK_WIDTH; ++ly)
    for(int lx=0; lx<CHUNK_WIDTH; ++lx)
    {
      int total_height = chunk_info.stone_height_map.heights[ly][lx]
                       + chunk_info.grass_height_map.heights[ly][lx];
      max_height = std::max(max_height, total_height);
    }

  chunk_data.slices.reserve(max_height);
  for(int lz=0; lz<max_height; ++lz)
  {
    ChunkData::Slice slice;
    for(int ly=0; ly<CHUNK_WIDTH; ++ly)
      for(int lx=0; lx<CHUNK_WIDTH; ++lx)
      {
        int height1 = chunk_info.stone_height_map.heights[ly][lx];
        int height2 = chunk_info.stone_height_map.heights[ly][lx] + chunk_info.grass_height_map.heights[ly][lx];
        slice.blocks[ly][lx] = lz < height1 ? Block::STONE :
                               lz < height2 ? Block::GRASS :
                                              Block::NONE;
      }
    chunk_data.slices.push_back(slice);
  }

  // 2: Carve out caves based off worms
  int        radius  = std::ceil(CAVE_WORM_SEGMENT_MAX * CAVE_WORM_STEP / CHUNK_WIDTH);
  glm::ivec2 corner1 = chunk_position - glm::ivec2(radius, radius);
  glm::ivec2 corner2 = chunk_position + glm::ivec2(radius, radius);
  for(int cy = corner1.y; cy <= corner2.y; ++cy)
    for(int cx = corner1.x; cx <= corner2.x; ++cx)
    {
      glm::ivec2 neighbour_chunk_position = glm::ivec2(cx, cy);

      auto it = chunk_manager.chunk_infos().find(neighbour_chunk_position);
      assert(it != chunk_manager.chunk_infos().end());

      const ChunkInfo& chunk_info = it->second;
      for(const Worm& worm : chunk_info.worms)
        for(const Worm::Node& node : worm.nodes)
          chunk_data.explode(global_to_local(node.center, chunk_position), node.radius);
    }

  return chunk_data;
}

