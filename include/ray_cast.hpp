#ifndef RAYCAST_HPP
#define RAYCAST_HPP

#include <glm/glm.hpp>

#include <optional>
#include <type_traits>

inline void ray_step(glm::ivec3& block_position, glm::vec3& position, glm::vec3 direction, float& length)
{
  float      t = std::numeric_limits<float>::infinity();
  glm::ivec3 step;
  for(int i=0; i<3; ++i)
  {
    float      _t;
    glm::ivec3 _step;
    if(direction[i] < 0.0f)
    {
      _t       = (block_position[i] - position[i]) / direction[i];
      _step    = {};
      _step[i] = -1;
    }
    else if(direction[i] > 0.0f)
    {
      _t       = (block_position[i] + 1 - position[i]) / direction[i];
      _step    = {};
      _step[i] = 1;
    }
    else
      continue;

    if(t > _t)
    {
      t    = _t;
      step = _step;
    }
  }

  block_position += step;
  position       += t * direction;
  length         -= t;
}

inline std::optional<glm::ivec3> ray_cast(glm::vec3 position, glm::vec3 direction, float length, auto callback) requires(std::is_invocable_r_v<bool, decltype(callback), glm::ivec3>)
{
  glm::ivec3 block_position = glm::floor(position);
  while(length >= 0.0f)
  {
    if(callback(block_position))
      return block_position;

    ray_step(block_position, position, direction, length);
  }
  return std::nullopt;
}

#endif // RAYCAST_HPP
