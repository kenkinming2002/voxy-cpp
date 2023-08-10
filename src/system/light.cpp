#include <system/light.hpp>

#include <world.hpp>

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

  void on_update(World& world, float dt) override
  {
    while(!world.dimension.pending_lighting_updates.empty())
    {
      glm::ivec3 position = *world.dimension.pending_lighting_updates.begin();
      world.dimension.pending_lighting_updates.erase(world.dimension.pending_lighting_updates.begin());

      Block* block = world.dimension.get_block(position);
      if(!block)
        continue;

      auto [new_sky, new_light_level] = compute_block_sky_and_light_level(world, position, block);
      if(block->sky != new_sky)
      {
        block->sky = new_sky;

        glm::ivec3 neighbour_position = position + glm::ivec3(0, 0, -1);
        world.dimension.lighting_invalidate(neighbour_position);
      }

      if(block->light_level != new_light_level)
      {
        block->light_level = new_light_level;
        for(glm::ivec3 direction : DIRECTIONS)
        {
          glm::ivec3 neighbour_position = position + direction;
          world.dimension.minor_invalidate_mesh(neighbour_position);
          world.dimension.lighting_invalidate(neighbour_position);
        }
      }
    }
  }
};

std::unique_ptr<System> create_light_system()
{
  return std::make_unique<LightSystem>();
}

