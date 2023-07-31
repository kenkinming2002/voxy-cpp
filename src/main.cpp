#include <gl.hpp>
#include <sdl2.hpp>
#include <timer.hpp>
#include <window.hpp>
#include <world.hpp>

#include <spdlog/spdlog.h>

#include <stdexcept>

static constexpr size_t SEED = 0b1011011010110101110110110101110101011010110101011111010100011010;

int main()
{
  Window window("voxy", 1024, 720);
  World  world(SEED);
  Timer  timer;

  bool running = true;
  while(running)
  {
    while(auto event = window.poll_event())
    {
      switch(event->type) {
        case SDL_KEYDOWN:
          if(event->key.keysym.sym == SDLK_ESCAPE)
            running = false;
          break;
        case SDL_QUIT:
          running = false;
          break;
      }
      world.handle_event(*event);
    }

    float dt = timer.tick();
    world.update(dt);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    world.render();
    window.swap_buffer();
  }
}

