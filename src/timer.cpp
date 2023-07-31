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

  for(size_t i=0; i<TIMER_AVERAGE_COUNT-1; ++i)
    m_dts[i] = m_dts[i+1];
  m_dts[TIMER_AVERAGE_COUNT-1] = dt;

  return dt;
}

float Timer::average() const
{
  float average = 0.0f;
  for(size_t i=0; i<TIMER_AVERAGE_COUNT; ++i)
    average += m_dts[i];
  average /= TIMER_AVERAGE_COUNT;
  return average;
}
