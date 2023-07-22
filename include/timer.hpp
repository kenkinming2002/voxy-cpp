#ifndef TIMER_HPP
#define TIMER_HPP

struct Timer
{
  Timer();
  float tick();

  float previous_time;
};

#endif // TIMER_HPP
