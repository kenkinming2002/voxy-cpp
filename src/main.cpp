#include <camera.hpp>
#include <gl.hpp>
#include <sdl2.hpp>
#include <mesh.hpp>
#include <timer.hpp>
#include <world.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <unordered_map>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <sstream>
#include <utility>

#include <algorithm>
#include <math.h>

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
    world.render();
    SDL_GL_SwapWindow(window);
  }
}
