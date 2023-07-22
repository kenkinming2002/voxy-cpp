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

#include <iostream>
#include <algorithm>
#include <math.h>

Camera camera;

bool first = true;
float last_xpos;
float last_ypos;

int main()
{
  sdl2::Context sdl2_context;
  sdl2::Window window("voxy", 800, 600);
  SDL_SetRelativeMouseMode(SDL_TRUE);

  if(!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    throw std::runtime_error("Failed to load OpenGL functions with GLAD");

  gl::init_debug();

  World world;

  world.generate_chunk     (glm::ivec3(0, 0, 0));
  world.generate_chunk_mesh(glm::ivec3(0, 0, 0));

  world.generate_chunk     (glm::ivec3(1, 1, 1));
  world.generate_chunk_mesh(glm::ivec3(1, 1, 1));

  gl::Texture texture0 = gl::load_texture("assets/container.jpg");
  gl::Texture texture1 = gl::load_texture("assets/awesomeface.png");
  gl::Program program = gl::compile_program("assets/shader.vert", "assets/shader.frag");

  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  Timer timer;

  bool running = true;
  while(running) {
    float dt = timer.tick();

    SDL_Event event;
    while(SDL_PollEvent(&event))
      switch(event.type) {
        case SDL_KEYDOWN:
          switch(event.key.keysym.sym) {
            case SDLK_ESCAPE: running = false; break;
          }
          break;
        case SDL_MOUSEMOTION:
          camera.rotate(-event.motion.xrel, -event.motion.yrel);
          break;
        case SDL_MOUSEWHEEL:
          camera.zoom(-event.wheel.y);
          break;
        case SDL_QUIT:
          running = false;
          break;
      }

    glm::vec3 translation = glm::vec3(0.0f);

    const Uint8 *keys = SDL_GetKeyboardState(nullptr);
    if(keys[SDL_SCANCODE_SPACE])  translation.z += 1.0f;
    if(keys[SDL_SCANCODE_LSHIFT]) translation.z -= 1.0f;
    if(keys[SDL_SCANCODE_W])      translation.y += 1.0f;
    if(keys[SDL_SCANCODE_S])      translation.y -= 1.0f;
    if(keys[SDL_SCANCODE_D])      translation.x += 1.0f;
    if(keys[SDL_SCANCODE_A])      translation.x -= 1.0f;
    if(glm::length(translation) != 0.0f)
    {
      translation = glm::normalize(translation);
      translation *= dt;
      camera.translate(translation.x, translation.y, translation.z);
    }

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);

    // Texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glUniform1i(glGetUniformLocation(program, "texture0"), 0);
    glUniform1i(glGetUniformLocation(program, "texture1"), 1);

    world.draw(camera);

    SDL_GL_SwapWindow(window);
  }
}
