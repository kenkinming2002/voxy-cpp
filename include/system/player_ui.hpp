#ifndef SYSTEM_PLAYER_UI_HPP
#define SYSTEM_PLAYER_UI_HPP

#include <glm/fwd.hpp>
#include <memory>
#include <stddef.h>

struct World;
class PlayerUISystem
{
public:
  static std::unique_ptr<PlayerUISystem> create();

public:
  virtual void render(const World& world) = 0;
  virtual ~PlayerUISystem() = default;
};

#endif // SYSTEM_PLAYER_UI_HPP


