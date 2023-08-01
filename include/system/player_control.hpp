#ifndef SYSTEM_PLAYER_MOVEMENT_HPP
#define SYSTEM_PLAYER_MOVEMENT_HPP

#include <SDL.h>

#include <memory>
#include <stddef.h>

struct World;
class PlayerControlSystem
{
public:
  static std::unique_ptr<PlayerControlSystem> create();

public:
  virtual void handle_event(World& world, SDL_Event event) = 0;
  virtual void update(World& world, float dt) = 0;
  virtual ~PlayerControlSystem() = default;
};

#endif // SYSTEM_PLAYER_MOVEMENT_HPP

