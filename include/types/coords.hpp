#ifndef CHUNK_COORDS_HPP
#define CHUNK_COORDS_HPP

#include <types/chunk.hpp>

#include <glm/glm.hpp>

template<glm::length_t L, typename T, glm::qualifier Q>
inline glm::vec<L, T, Q> global_to_local(glm::vec<L, T, Q> position, glm::ivec2 chunk_index)
{
  position[0] -= chunk_index[0] * CHUNK_WIDTH;
  position[1] -= chunk_index[1] * CHUNK_WIDTH;
  return position;
}

template<glm::length_t L, typename T, glm::qualifier Q>
inline glm::vec<L, T, Q> local_to_global(glm::vec<L, T, Q> position, glm::ivec2 chunk_index)
{
  position[0] += chunk_index[0] * CHUNK_WIDTH;
  position[1] += chunk_index[1] * CHUNK_WIDTH;
  return position;
}

#endif // CHUNK_COORDS_HPP
