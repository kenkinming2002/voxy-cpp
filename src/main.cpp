#include <gl.hpp>
#include <sdl2.hpp>
#include <timer.hpp>
#include <world.hpp>

#include <spdlog/spdlog.h>

#include <stdexcept>

static constexpr size_t SEED = 0b1011011010110101110110110101110101011010110101011111010100011010;

int main()
{
  sdl2::Context sdl2_context;
  sdl2::Window window("voxy", 1024, 720);
  SDL_SetRelativeMouseMode(SDL_TRUE);

  if(!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    throw std::runtime_error("Failed to load OpenGL functions with GLAD");

  gl::init_debug();

  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ALIGNMENT,   1);

  World world(SEED);
  Timer timer;

  bool running = true;
  while(running) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      switch(event.type) {
        case SDL_KEYDOWN:
          if(event.key.keysym.sym == SDLK_ESCAPE)
            running = false;
          break;
        case SDL_QUIT:
          running = false;
          break;
      }
      world.handle_event(event);
    }

    float dt = timer.tick();
    world.update(dt);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    world.render();
    SDL_GL_SwapWindow(window);
  }
}

