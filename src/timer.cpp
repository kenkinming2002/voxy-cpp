#include <timer.hpp>

Timer::Timer()
{
  m_previous_tick = SDL_GetTicks();
}

float Timer::tick()
{
  Uint32 tick = SDL_GetTicks();
  float dt = (float)(tick - m_previous_tick) / 1000.0f;
  m_previous_tick = tick;
  return dt;
}

