#pragma once

#include <camera.hpp>
#include <dimension.hpp>
#include <entity.hpp>
#include <world_config.hpp>

#include <graphics/texture_array.hpp>
#include <graphics/mesh.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <optional>

#include <cstddef>

struct WorldData
{
public:
  EntityData    player;
  DimensionData dimension;

  std::optional<glm::ivec3> placement;
  std::optional<glm::ivec3> selection;

public:
  Block* get_block(glm::ivec3 position);
  const Block* get_block(glm::ivec3 position) const;

  void invalidate_mesh_major(glm::ivec3 position);
  void invalidate_mesh_minor(glm::ivec3 position);

  void invalidate_light(glm::ivec3 position);
};

