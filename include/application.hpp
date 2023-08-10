#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <timer.hpp>
#include <window.hpp>

#include <world.hpp>
#include <system.hpp>

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

private:
  Window m_window;
  Timer  m_timer;

private:
  World  m_world;
  std::vector<std::unique_ptr<System>> m_systems;
};

#endif // APPLICATION_HPP
