#include <light_manager.hpp>

void LightManager::invalidate(glm::ivec3 position)
{
  m_invalidations.emplace(position, Invalidation{});
}

void LightManager::update(World& world)
{
  std::unordered_set<glm::ivec3>               updates;
  std::unordered_map<glm::ivec3, Invalidation> new_invalidations;
  while(!m_invalidations.empty())
  {
    /*************
     * 1: Update *
     *************/
    for(auto& [position, invalidation] : m_invalidations)
    {
      invalidation.block = get_block(world, position);
      if(!invalidation.block)
        continue;

      // 1: Solid block
      if(invalidation.block->id != BLOCK_ID_NONE)
      {
        invalidation.new_sky         = false;
        invalidation.new_light_level = 0;
        continue;
      }

      // 2: Direct Skylight
      if(position.z == CHUNK_HEIGHT - 1)
      {
        invalidation.new_sky         = true;
        invalidation.new_light_level = 15;
        continue;
      }

      // 3: Indirect Skylight
      if(const Block* neighbour_block = get_block(world, position + glm::ivec3(0, 0, 1)); neighbour_block->sky)
      {
        invalidation.new_sky         = true;
        invalidation.new_light_level = 15;
        continue;
      }

      // 4: Neighbours
      int light_level_max = 0;
      { const Block* neighbour_block = get_block(world, position + glm::ivec3(-1, 0, 0)); light_level_max = std::max<int>(light_level_max, neighbour_block ? neighbour_block->light_level : 15); if(light_level_max == 15) goto end; }
      { const Block* neighbour_block = get_block(world, position + glm::ivec3( 1, 0, 0)); light_level_max = std::max<int>(light_level_max, neighbour_block ? neighbour_block->light_level : 15); if(light_level_max == 15) goto end; }
      { const Block* neighbour_block = get_block(world, position + glm::ivec3(0, -1, 0)); light_level_max = std::max<int>(light_level_max, neighbour_block ? neighbour_block->light_level : 15); if(light_level_max == 15) goto end; }
      { const Block* neighbour_block = get_block(world, position + glm::ivec3(0,  1, 0)); light_level_max = std::max<int>(light_level_max, neighbour_block ? neighbour_block->light_level : 15); if(light_level_max == 15) goto end; }
      { const Block* neighbour_block = get_block(world, position + glm::ivec3(0, 0, -1)); light_level_max = std::max<int>(light_level_max, neighbour_block ? neighbour_block->light_level : 15); if(light_level_max == 15) goto end; }
      { const Block* neighbour_block = get_block(world, position + glm::ivec3(0, 0,  1)); light_level_max = std::max<int>(light_level_max, neighbour_block ? neighbour_block->light_level : 15); if(light_level_max == 15) goto end; }
end:

      invalidation.new_sky         = false;
      invalidation.new_light_level = light_level_max > 0 ? light_level_max - 1 : 0;
    }

    /*************
     * 2: commit *
     *************/
    for(auto& [position, invalidation] : m_invalidations)
      if(invalidation.block)
      {
        if(invalidation.block->sky != invalidation.new_sky)
        {
          invalidation.block->sky = invalidation.new_sky;
          new_invalidations.emplace(position + glm::ivec3(0, 0, -1), Invalidation{});
        }

        if(invalidation.block->light_level != invalidation.new_light_level)
        {
          invalidation.block->light_level = invalidation.new_light_level;
          new_invalidations.emplace(position + glm::ivec3(-1, 0, 0), Invalidation{});
          new_invalidations.emplace(position + glm::ivec3( 1, 0, 0), Invalidation{});
          new_invalidations.emplace(position + glm::ivec3(0, -1, 0), Invalidation{});
          new_invalidations.emplace(position + glm::ivec3(0,  1, 0), Invalidation{});
          new_invalidations.emplace(position + glm::ivec3(0, 0, -1), Invalidation{});
          new_invalidations.emplace(position + glm::ivec3(0, 0,  1), Invalidation{});
          updates.insert(position);
        }
      }


    // 3: Iterate
    m_invalidations = std::move(new_invalidations);
    new_invalidations.clear();
  }

  for(glm::ivec3 update : updates)
  {
    invalidate_mesh(world, update + glm::ivec3(-1, 0, 0));
    invalidate_mesh(world, update + glm::ivec3( 1, 0, 0));
    invalidate_mesh(world, update + glm::ivec3(0, -1, 0));
    invalidate_mesh(world, update + glm::ivec3(0,  1, 0));
    invalidate_mesh(world, update + glm::ivec3(0, 0, -1));
    invalidate_mesh(world, update + glm::ivec3(0, 0,  1));
  }
}

