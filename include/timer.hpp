#ifndef TIMER_HPP
#define TIMER_HPP

#include <SDL2/SDL.h>

struct Timer
{
  Timer();
  float tick();

  Uint32 previous_tick;
};

#endif // TIMER_HPP
