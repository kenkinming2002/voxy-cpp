#include <system/light.hpp>

#include <world.hpp>
#include <coordinates.hpp>
#include <directions.hpp>

#include <spdlog/spdlog.h>

#include <unordered_set>

class LightSystem : public System
{
private:
  static std::pair<bool, int> compute_block_sky_and_light_level(const World& world, glm::ivec3 position, const Block* block)
  {
    glm::ivec3   neighbour_position;
    const Block* neighbour_block;

    // 1: Solid block
    if(block->id != Block::ID_NONE)
      return std::make_pair(false, 0);

    // 2: Direct Skylight
    if(position.z == Chunk::HEIGHT - 1)
      return std::make_pair(true, 15);

    // 3: Indirect Skylight
    neighbour_position = position + glm::ivec3(0, 0, 1);
    neighbour_block    = world.dimension.get_block(neighbour_position);
    if(neighbour_block->sky)
      return std::make_pair(true, 15);

    // 4: Neighbours
    int light_level = 0;
    for(glm::ivec3 direction : DIRECTIONS)
    {
      neighbour_position = position + direction;
      neighbour_block    = world.dimension.get_block(neighbour_position);
      light_level = std::max(light_level, neighbour_block ? (int)neighbour_block->light_level : 15);
      if(light_level == 15)
        break;
    }
    return std::make_pair(false, std::max(light_level-1, 0));
  }

  void on_update(Application& application, World& world, float dt) override
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
        for(glm::vec3 position : chunk.pending_lighting_updates)
          items.push_back(Item{.position = coordinates::local_to_global(position, chunk_index)});
        chunk.pending_lighting_updates.clear();
      }
      if(items.empty())
        break;

      // 2: Update
#pragma omp parallel for
      for(Item& item : items)
      {
        item.block = world.dimension.get_block(item.position);
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
            world.dimension.lighting_invalidate(neighbour_position);
          }

          if(item.block->light_level != item.new_light_level)
          {
            item.block->light_level = item.new_light_level;
            for(glm::ivec3 direction : DIRECTIONS)
            {
              glm::ivec3 neighbour_position = item.position + direction;
              world.dimension.minor_invalidate_mesh(neighbour_position);
              world.dimension.lighting_invalidate(neighbour_position);
            }
          }
        }
    }
  }
};

std::unique_ptr<System> create_light_system()
{
  return std::make_unique<LightSystem>();
}

