#include <timer.hpp>

Timer::Timer()
{
  previous_tick = SDL_GetTicks();
}

float Timer::tick()
{
  Uint32 tick = SDL_GetTicks();
  float dt = (float)(tick - previous_tick) / 1000.0f;
  previous_tick = tick;
  return dt;
}
