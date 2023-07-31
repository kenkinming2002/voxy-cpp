#include <system/light.hpp>

#include <types/world.hpp>
#include <types/directions.hpp>

#include <unordered_set>

static constexpr size_t LIGHTING_UPDATE_PER_FRAME = 20000;

class LightSystemImpl : public LightSystem
{
private:
  void update(World& world) override
  {
    for(unsigned i=0; i<LIGHTING_UPDATE_PER_FRAME; ++i)
    {
      if(world.dimension.pending_lighting_updates.empty())
        return;

      glm::ivec3 position = *world.dimension.pending_lighting_updates.begin();
      world.dimension.pending_lighting_updates.erase(world.dimension.pending_lighting_updates.begin());

      Block* block = world.dimension.get_block(position);
      if(!block)
        continue;

      // 1: Skylight
      int sky_light_level = 15;
      for(int z=position.z+1; z<CHUNK_HEIGHT; ++z)
      {
        glm::ivec3 neighbour_position = { position.x, position.y, z };
        Block*     neighbour_block    = world.dimension.get_block(neighbour_position);
        if(neighbour_block->presence) // TODO: Opaqueness
        {
          sky_light_level = 0;
          break;
        }
      }

      // 2: TODO: Emitters

      // 3: Neighbours
      int neighbour_light_level = 0;
      for(glm::ivec3 direction : DIRECTIONS)
      {
        glm::ivec3 neighbour_position = position + direction;
        Block*     neighbour_block    = world.dimension.get_block(neighbour_position);
        neighbour_light_level = std::max<int>(neighbour_light_level, neighbour_block ? neighbour_block->light_level : 15);
      }
      neighbour_light_level = std::max(neighbour_light_level-1, 0);

      int new_light_level = std::max(sky_light_level, neighbour_light_level);
      if(block->light_level != new_light_level)
      {
        block->light_level = new_light_level;
        world.dimension.minor_invalidate_mesh(position);
        for(glm::ivec3 direction : DIRECTIONS)
        {
          glm::ivec3 neighbour_position = position + direction;
          world.dimension.pending_lighting_updates.insert(neighbour_position);
        }
      }
    }
  }
};

std::unique_ptr<LightSystem> LightSystem::create()
{
  return std::make_unique<LightSystemImpl>();
}

