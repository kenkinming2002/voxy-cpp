#include <timer.hpp>

#include <GLFW/glfw3.h>

Timer::Timer()
{
  previous_time = glfwGetTime();
}

float Timer::tick()
{
  float time = glfwGetTime();
  float dt = time - previous_time;
  previous_time = time;
  return dt;
}
