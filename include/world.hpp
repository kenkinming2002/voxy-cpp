#pragma once

#include <camera.hpp>
#include <entity.hpp>
#include <dimension.hpp>

#include <graphics/texture_array.hpp>
#include <graphics/mesh.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <optional>

#include <cstddef>

struct World
{
public:
  std::size_t seed;

  Camera    camera;
  Entity    player;
  Dimension dimension;

  std::optional<glm::ivec3> placement;
  std::optional<glm::ivec3> selection;

public:
  Block* get_block(glm::ivec3 position);
  const Block* get_block(glm::ivec3 position) const;

  void invalidate_mesh_major(glm::ivec3 position);
  void invalidate_mesh_minor(glm::ivec3 position);

  void invalidate_light(glm::ivec3 position);
};

