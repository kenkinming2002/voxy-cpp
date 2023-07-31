#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <timer.hpp>
#include <window.hpp>
#include <world.hpp>

class Application
{
public:
  Application();

public:
  void run();

private:
  void loop();

private:
  bool m_running;

  Window m_window;
  Timer  m_timer;
  World  m_world;
};

#endif // APPLICATION_HPP
