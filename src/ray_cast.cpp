#include <ray_cast.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

RayCastBlocksResult ray_cast_blocks(const World& world, glm::vec3 position, glm::vec3 direction, float length)
{
  glm::ivec3 iposition = glm::floor(position);

  // 1: Check if we are inside a block already
  if(const Block * block = get_block(world, iposition); block && block->id != BLOCK_ID_NONE)
  {
    RayCastBlocksResult result = {};
    result.type     = RayCastBlocksResult::Type::INSIDE_BLOCK;
    result.position = iposition;
    return result;
  }

  // 2: Special case if direction is zero, so that we cannot actually step in any directions
  if(direction == glm::vec3(0.0f))
  {
    RayCastBlocksResult result = {};
    result.type = RayCastBlocksResult::Type::NONE;
    return result;
  }

  for(;;)
  {
    // 3: Find the direction to step in
    float min_t = std::numeric_limits<float>::infinity();
    int min_step;
    int min_i;
    for(size_t i=0; i<3; ++i)
    {
      int step;
      int target;
      if(direction[i] < 0.0f)
      {
        step   = -1;
        target = iposition[i];
      }
      else if(direction[i] > 0.0f)
      {
        step   = 1;
        target = iposition[i] + 1;
      }
      else
        continue;

      float t = (target - position[i]) / direction[i];
      if(min_t > t)
      {
        min_t = t;
        min_step = step;
        min_i    = i;
      }
    }

    // 4: Check if the ray is actually long enough to step in that direction
    if(length < min_t)
    {
      RayCastBlocksResult result = {};
      result.type = RayCastBlocksResult::Type::NONE;
      return result;
    }

    // 5: Step and see if we hit a block
    iposition[min_i] += min_step;
    position += min_t * direction;
    length   -= min_t;
    if(const Block * block = get_block(world, iposition); block && block->id != BLOCK_ID_NONE)
    {
      RayCastBlocksResult result = {};
      result.type          = RayCastBlocksResult::Type::HIT;
      result.position      = iposition;
      result.normal        = glm::ivec3(0);
      result.normal[min_i] = -min_step;
      return result;
    }
  }
}

#pragma GCC diagnostic pop
