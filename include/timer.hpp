#ifndef TIMER_HPP
#define TIMER_HPP

#include <SDL2/SDL.h>

static constexpr size_t TIMER_AVERAGE_COUNT = 32;
struct Timer
{
public:
  Timer();

public:
  float tick();

private:
  Uint32 m_previous_tick;
};

#endif // TIMER_HPP
