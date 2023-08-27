#pragma once

class Timer
{
public:
  Timer();

public:
  bool tick(float dt);

private:
  float m_previous_time;
};
