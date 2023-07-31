#include <chunk/light_system.hpp>

#include <dimension.hpp>

#include <unordered_set>

static constexpr size_t LIGHTING_UPDATE_PER_FRAME = 20000;

class ChunkLightSystemImpl : public ChunkLightSystem
{
private:
  void update(Dimension& dimension) override
  {
    for(unsigned i=0; i<LIGHTING_UPDATE_PER_FRAME; ++i)
    {
      if(dimension.pending_lighting_updates().empty())
        return;

      glm::ivec3 position = *dimension.pending_lighting_updates().begin();
      dimension.pending_lighting_updates().erase(dimension.pending_lighting_updates().begin());

      std::optional<Block> block = dimension.get_block(position);
      if(!block)
        continue;

      // 1: Skylight
      int sky_light_level = 15;
      for(int z=position.z+1; z<CHUNK_HEIGHT; ++z)
      {
        glm::ivec3 neighbour_position = { position.x, position.y, z };
        Block      neighbour_block    = dimension.get_block(neighbour_position).value();
        if(neighbour_block.presence) // TODO: Opaqueness
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
        Block      neighbour_block    = dimension.get_block(neighbour_position).value_or(Block{.light_level = 15});
        neighbour_light_level = std::max<int>(neighbour_light_level, neighbour_block.light_level);
      }
      neighbour_light_level = std::max(neighbour_light_level-1, 0);

      int new_light_level = std::max(sky_light_level, neighbour_light_level);
      if(block->light_level != new_light_level)
      {
        block->light_level = new_light_level;
        dimension.set_block(position, *block);
        dimension.minor_invalidate_mesh(position);
        for(glm::ivec3 direction : DIRECTIONS)
        {
          glm::ivec3 neighbour_position = position + direction;
          dimension.pending_lighting_updates().insert(neighbour_position);
        }
      }
    }
  }
};

std::unique_ptr<ChunkLightSystem> ChunkLightSystem::create()
{
  return std::make_unique<ChunkLightSystemImpl>();
}

