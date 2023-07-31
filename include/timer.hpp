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
  float average() const;

private:
  Uint32 m_previous_tick;
  float m_dts[TIMER_AVERAGE_COUNT];
};

#endif // TIMER_HPP
