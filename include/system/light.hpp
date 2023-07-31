#ifndef LIGHT_SYSTEM_HPP
#define LIGHT_SYSTEM_HPP

#include <glm/fwd.hpp>
#include <memory>
#include <stddef.h>

struct World;
class LightSystem
{
public:
  static std::unique_ptr<LightSystem> create();

public:
  virtual void update(World& world) = 0;
  virtual ~LightSystem() = default;
};

#endif // LIGHT_SYSTEM_HPP
