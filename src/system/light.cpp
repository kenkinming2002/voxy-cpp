#include <system/light.hpp>

#include <types/world.hpp>
#include <types/directions.hpp>

#include <unordered_set>

static constexpr size_t LIGHTING_UPDATE_PER_FRAME = 100000;

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

      int old_light_level = block->light_level;

      // 1: Solid block
      if(block->presence) // TODO: Check for opaqueness
      {
        block->sky         = false;
        block->light_level = 0;
        goto done;
      }

      // 2: Skylight
      if(position.z == CHUNK_HEIGHT - 1)
      {
        block->sky         = true;
        block->light_level = 15;
      }
      else
      {
        glm::ivec3 neighbour_position = position + glm::ivec3(0, 0, 1);
        Block*     neighbour_block    = world.dimension.get_block(neighbour_position);
        if(neighbour_block->sky)
        {
          block->sky         = true;
          block->light_level = 15;
        }
      }

      if(block->light_level == 15)
        goto done;

      // 3: Neighbours
      for(glm::ivec3 direction : DIRECTIONS)
      {
        glm::ivec3 neighbour_position = position + direction;
        Block*     neighbour_block    = world.dimension.get_block(neighbour_position);

        int neighbour_light_level;
        if(neighbour_block)
        {
          if(neighbour_block->light_level != 0)
            neighbour_light_level = (int)neighbour_block->light_level - 1;
          else
            neighbour_light_level = 0;
        }
        else
            neighbour_light_level = 15;

        block->light_level = std::max<int>(block->light_level, neighbour_light_level);
      }

done:
      if(block->light_level != old_light_level)
      {
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

std::unique_ptr<LightSystem> LightSystem::create()
{
  return std::make_unique<LightSystemImpl>();
}

