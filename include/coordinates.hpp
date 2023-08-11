#pragma once

#include <world.hpp>

#include <glm/glm.hpp>

namespace coordinates
{
  namespace details
  {
    inline int modulo(int a, int b)
    {
      int value = a % b;
      if(value < 0)
        value += b;

      assert(0 <= value);
      assert(value < b);
      return value;
    }
  }

  template<glm::length_t L, typename T, glm::qualifier Q>
  inline glm::vec<L, T, Q> global_to_local(glm::vec<L, T, Q> position, glm::ivec2 chunk_index)
  {
    position[0] -= chunk_index[0] * Chunk::WIDTH;
    position[1] -= chunk_index[1] * Chunk::WIDTH;
    return position;
  }

  template<glm::length_t L, typename T, glm::qualifier Q>
  inline glm::vec<L, T, Q> local_to_global(glm::vec<L, T, Q> position, glm::ivec2 chunk_index)
  {
    position[0] += chunk_index[0] * Chunk::WIDTH;
    position[1] += chunk_index[1] * Chunk::WIDTH;
    return position;
  }

  template<glm::length_t L, glm::qualifier Q>
  inline std::pair<glm::vec<L, int, Q>, glm::ivec2> split(glm::vec<L, int, Q> position)
  {
    glm::vec<L, int, Q> local_position = position;
    local_position[0] = details::modulo(local_position[0], Chunk::WIDTH);
    local_position[1] = details::modulo(local_position[1], Chunk::WIDTH);

    glm::ivec2 chunk_index = glm::ivec2(
      (position[0] - local_position[0]) / Chunk::WIDTH,
      (position[1] - local_position[1]) / Chunk::WIDTH
    );

    return std::make_pair(local_position, chunk_index);
  }
}
