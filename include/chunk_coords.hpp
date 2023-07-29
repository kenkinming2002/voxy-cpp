#ifndef CHUNK_COORDS_HPP
#define CHUNK_COORDS_HPP

#include <chunk_defs.hpp>

#include <glm/glm.hpp>

inline glm::ivec3 global_to_local(glm::ivec3 position, glm::ivec2 chunk_position)
{
  return glm::ivec3(
      position.x - chunk_position.x * CHUNK_WIDTH,
      position.y - chunk_position.y * CHUNK_WIDTH,
      position.z
  );
}

inline glm::vec3 global_to_local(glm::vec3 position, glm::ivec2 chunk_position)
{
  return glm::vec3(
      position.x - chunk_position.x * CHUNK_WIDTH,
      position.y - chunk_position.y * CHUNK_WIDTH,
      position.z
  );
}

inline glm::ivec3 local_to_global(glm::ivec3 position, glm::ivec2 chunk_position)
{
  return glm::ivec3(
      position.x + chunk_position.x * CHUNK_WIDTH,
      position.y + chunk_position.y * CHUNK_WIDTH,
      position.z
  );
}

inline glm::vec3 local_to_global(glm::vec3 position, glm::ivec2 chunk_position)
{
  return glm::vec3(
      position.x + chunk_position.x * CHUNK_WIDTH,
      position.y + chunk_position.y * CHUNK_WIDTH,
      position.z
  );
}

inline glm::ivec2 global_to_local(glm::ivec2 position, glm::ivec2 chunk_position)
{
  return glm::ivec2(
      position.x - chunk_position.x * CHUNK_WIDTH,
      position.y - chunk_position.y * CHUNK_WIDTH
  );
}

inline glm::vec2 global_to_local(glm::vec2 position, glm::ivec2 chunk_position)
{
  return glm::vec2(
      position.x - chunk_position.x * CHUNK_WIDTH,
      position.y - chunk_position.y * CHUNK_WIDTH
  );
}

inline glm::ivec2 local_to_global(glm::ivec2 position, glm::ivec2 chunk_position)
{
  return glm::ivec2(
      position.x + chunk_position.x * CHUNK_WIDTH,
      position.y + chunk_position.y * CHUNK_WIDTH
  );
}

inline glm::vec2 local_to_global(glm::vec2 position, glm::ivec2 chunk_position)
{
  return glm::vec2(
      position.x + chunk_position.x * CHUNK_WIDTH,
      position.y + chunk_position.y * CHUNK_WIDTH
  );
}


#endif // CHUNK_COORDS_HPP
