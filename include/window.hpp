#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <sdl2.hpp>

#include <optional>

class Window
{
public:
  Window(const char *name, unsigned width, unsigned height);

public:
  std::optional<SDL_Event> poll_event();
  void swap_buffer();

private:
  sdl2::Context m_context;
  sdl2::Window  m_window;
};

#endif // WINDOW_HPP
