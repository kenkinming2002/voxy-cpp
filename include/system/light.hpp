#ifndef LIGHT_SYSTEM_HPP
#define LIGHT_SYSTEM_HPP

#include <glm/fwd.hpp>
#include <memory>
#include <stddef.h>

class Dimension;
class LightSystem
{
public:
  static std::unique_ptr<LightSystem> create();

public:
  virtual void update(Dimension& dimension) = 0;
  virtual ~LightSystem() = default;
};

#endif // LIGHT_SYSTEM_HPP
