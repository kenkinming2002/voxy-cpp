#ifndef SYSTEM_DEBUG_HPP
#define SYSTEM_DEBUG_HPP

#include <memory>
#include <stddef.h>

struct World;
class DebugSystem
{
public:
  static std::unique_ptr<DebugSystem> create();

public:
  virtual void render(const World& world) = 0;
  virtual ~DebugSystem() = default;
};

#endif // SYSTEM_DEBUG_HPP

