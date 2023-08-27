#include <timer.hpp>

#include <GLFW/glfw3.h>
#include <assert.h>

Timer::Timer()
{
  m_previous_time = glfwGetTime();
}

bool Timer::tick(float dt)
{
  float time = glfwGetTime();
  assert(time >= m_previous_time);
  if(time >= m_previous_time + dt)
  {
    m_previous_time += dt;
    return true;
  }
  else
    return false;
}
