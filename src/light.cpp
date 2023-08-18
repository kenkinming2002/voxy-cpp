#include <light.hpp>

#include <coordinates.hpp>
#include <directions.hpp>

static std::pair<bool, int> compute_block_sky_and_light_level(const World& world, glm::ivec3 position, const Block* block)
{
  glm::ivec3   neighbour_position;
  const Block* neighbour_block;

  // 1: Solid block
  if(block->id != BLOCK_ID_NONE)
    return std::make_pair(false, 0);

  // 2: Direct Skylight
  if(position.z == CHUNK_HEIGHT - 1)
    return std::make_pair(true, 15);

  // 3: Indirect Skylight
  neighbour_position = position + glm::ivec3(0, 0, 1);
  neighbour_block    = get_block(world.dimension, neighbour_position);
  if(neighbour_block->sky)
    return std::make_pair(true, 15);

  // 4: Neighbours
  int light_level = 0;
  for(glm::ivec3 direction : DIRECTIONS)
  {
    neighbour_position = position + direction;
    neighbour_block    = get_block(world.dimension, neighbour_position);
    light_level = std::max(light_level, neighbour_block ? (int)neighbour_block->light_level : 15);
    if(light_level == 15)
      break;
  }
  return std::make_pair(false, std::max(light_level-1, 0));
}

void update_light(World& world)
{
  struct Item
  {
    glm::ivec3   position;
    Block*       block;
    std::uint8_t new_sky;
    std::uint8_t new_light_level;
  };

  for(;;)
  {
    // 1: Prepare items
    std::vector<Item> items;
    for(auto& [chunk_index, chunk] : world.dimension.chunks)
    {
      for(glm::vec3 position : chunk.light_invalidations)
        items.push_back(Item{.position = coordinates::local_to_global(position, chunk_index)});
      chunk.light_invalidations.clear();
    }
    if(items.empty())
      break;

    // 2: Update
#pragma omp parallel for
    for(Item& item : items)
    {
      item.block = get_block(world.dimension, item.position);
      if(!item.block)
        continue;

      std::tie(item.new_sky, item.new_light_level) = compute_block_sky_and_light_level(world, item.position, item.block);
    }

    // 3: Commit
    for(Item& item : items)
      if(item.block)
      {
        if(item.block->sky != item.new_sky)
        {
          item.block->sky = item.new_sky;

          glm::ivec3 neighbour_position = item.position + glm::ivec3(0, 0, -1);
          invalidate_light(world, neighbour_position);
        }

        if(item.block->light_level != item.new_light_level)
        {
          item.block->light_level = item.new_light_level;
          for(glm::ivec3 direction : DIRECTIONS)
          {
            glm::ivec3 neighbour_position = item.position + direction;
            invalidate_mesh(world, neighbour_position);
            invalidate_light     (world, neighbour_position);
          }
        }
      }
  }

}
