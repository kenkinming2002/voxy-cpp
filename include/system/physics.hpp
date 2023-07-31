#ifndef SYSTEM_PHYSICS_HPP
#define SYSTEM_PHYSICS_HPP

#include <memory>
#include <stddef.h>

struct World;
class PhysicsSystem
{
public:
  static std::unique_ptr<PhysicsSystem> create();

public:
  virtual void update(World& world, float dt) = 0;
  virtual ~PhysicsSystem() = default;
};

#endif // SYSTEM_PHYSICS_HPP
