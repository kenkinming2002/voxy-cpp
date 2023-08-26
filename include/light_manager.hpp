#pragma once

#include <world.hpp>

class LightManager
{
public:
  void invalidate(glm::ivec3 position);
  void update(World& world);

private:
  struct Invalidation
  {
    Block*       block;
    std::uint8_t new_sky;
    std::uint8_t new_light_level;
  };
  std::unordered_map<glm::ivec3, Invalidation> m_invalidations;
};

