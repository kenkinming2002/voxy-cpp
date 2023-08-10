#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <SDL.h>

struct World;
class System
{
public:
  virtual void on_event (World& world, SDL_Event event) {};
  virtual void on_update(World& world, float dt)        {};
  virtual void on_render(const World& world)            {};
  virtual ~System() = default;
};

#endif // SYSTEM_HPP
