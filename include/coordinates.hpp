#pragma once

#include <world.hpp>

#include <glm/glm.hpp>

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

