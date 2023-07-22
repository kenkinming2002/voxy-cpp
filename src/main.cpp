#include <camera.hpp>
#include <gl.hpp>
#include <sdl2.hpp>
#include <mesh.hpp>
#include <timer.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdexcept>
#include <vector>
#include <fstream>
#include <sstream>
#include <utility>

#include <algorithm>
#include <math.h>

Camera camera;

bool first = true;
float last_xpos;
float last_ypos;

static const Vertex vertices[] = {
  {{-0.5f, -0.5f, -0.5f,}, { 0.0f, 0.0f,}, {1.0f, 1.0f, 1.0f}},
  {{0.5f, -0.5f, -0.5f, }, {1.0f, 0.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f,  0.5f, -0.5f, }, {1.0f, 1.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f,  0.5f, -0.5f, }, {1.0f, 1.0f, }, {1.0f, 1.0f, 1.0f}},
  {{-0.5f,  0.5f, -0.5f,}, { 0.0f, 1.0f,}, {1.0f, 1.0f, 1.0f}},
  {{-0.5f, -0.5f, -0.5f,}, { 0.0f, 0.0f,}, {1.0f, 1.0f, 1.0f}},

  {{-0.5f, -0.5f,  0.5f,}, { 0.0f, 0.0f,}, {1.0f, 1.0f, 1.0f}},
  {{0.5f, -0.5f,  0.5f, }, {1.0f, 0.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f,  0.5f,  0.5f, }, {1.0f, 1.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f,  0.5f,  0.5f, }, {1.0f, 1.0f, }, {1.0f, 1.0f, 1.0f}},
  {{-0.5f,  0.5f,  0.5f,}, { 0.0f, 1.0f,}, {1.0f, 1.0f, 1.0f}},
  {{-0.5f, -0.5f,  0.5f,}, { 0.0f, 0.0f,}, {1.0f, 1.0f, 1.0f}},

  {{-0.5f,  0.5f,  0.5f,}, { 1.0f, 0.0f,}, {1.0f, 1.0f, 1.0f}},
  {{-0.5f,  0.5f, -0.5f,}, { 1.0f, 1.0f,}, {1.0f, 1.0f, 1.0f}},
  {{-0.5f, -0.5f, -0.5f,}, { 0.0f, 1.0f,}, {1.0f, 1.0f, 1.0f}},
  {{-0.5f, -0.5f, -0.5f,}, { 0.0f, 1.0f,}, {1.0f, 1.0f, 1.0f}},
  {{-0.5f, -0.5f,  0.5f,}, { 0.0f, 0.0f,}, {1.0f, 1.0f, 1.0f}},
  {{-0.5f,  0.5f,  0.5f,}, { 1.0f, 0.0f,}, {1.0f, 1.0f, 1.0f}},

  {{0.5f,  0.5f,  0.5f, }, {1.0f, 0.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f,  0.5f, -0.5f, }, {1.0f, 1.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f, -0.5f, -0.5f, }, {0.0f, 1.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f, -0.5f, -0.5f, }, {0.0f, 1.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f, -0.5f,  0.5f, }, {0.0f, 0.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f,  0.5f,  0.5f, }, {1.0f, 0.0f, }, {1.0f, 1.0f, 1.0f}},

  {{-0.5f, -0.5f, -0.5f,}, { 0.0f, 1.0f,}, {1.0f, 1.0f, 1.0f}},
  {{0.5f, -0.5f, -0.5f, }, {1.0f, 1.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f, -0.5f,  0.5f, }, {1.0f, 0.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f, -0.5f,  0.5f, }, {1.0f, 0.0f, }, {1.0f, 1.0f, 1.0f}},
  {{-0.5f, -0.5f,  0.5f,}, { 0.0f, 0.0f,}, {1.0f, 1.0f, 1.0f}},
  {{-0.5f, -0.5f, -0.5f,}, { 0.0f, 1.0f,}, {1.0f, 1.0f, 1.0f}},

  {{-0.5f,  0.5f, -0.5f,}, { 0.0f, 1.0f,}, {1.0f, 1.0f, 1.0f}},
  {{0.5f,  0.5f, -0.5f, }, {1.0f, 1.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f,  0.5f,  0.5f, }, {1.0f, 0.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f,  0.5f,  0.5f, }, {1.0f, 0.0f, }, {1.0f, 1.0f, 1.0f}},
  {{-0.5f,  0.5f,  0.5f,}, { 0.0f, 0.0f,}, {1.0f, 1.0f, 1.0f}},
  {{-0.5f,  0.5f, -0.5f,}, { 0.0f, 1.0f }, {1.0f, 1.0f, 1.0f}},
};

static const uint16_t indices[] = {
  0, 1, 2, 3, 4, 5,
  6, 7, 8, 9, 10, 11,
  12, 13, 14, 15, 16, 17,
  18, 19, 20, 21, 22, 23,
  24, 25, 26, 27, 28, 29,
  30, 31, 32, 33, 34, 35,
};


int main()
{
  sdl2::Context sdl2_context;
  sdl2::Window window("voxy", 800, 600);
  SDL_SetRelativeMouseMode(SDL_TRUE);

  if(!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    throw std::runtime_error("Failed to load OpenGL functions with GLAD");

  gl::init_debug();

  glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f),
    glm::vec3( 2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3( 2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3( 1.3f, -2.0f, -2.5f),
    glm::vec3( 1.5f,  2.0f, -2.5f),
    glm::vec3( 1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
  };

  Mesh mesh(indices, vertices);
  gl::Texture texture0 = gl::load_texture("assets/container.jpg");
  gl::Texture texture1 = gl::load_texture("assets/awesomeface.png");
  gl::Program program = gl::compile_program("assets/shader.vert", "assets/shader.frag");

  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_DEPTH_TEST);

  Timer timer;

  bool up    = false;
  bool down  = false;
  bool right = false;
  bool left  = false;
  bool running = true;
  while(running) {
    float dt = timer.tick();

    SDL_Event event;
    while(SDL_PollEvent(&event))
      switch(event.type) {
        case SDL_KEYDOWN:
          switch(event.key.keysym.sym) {
            case SDLK_w: up     = true; break;
            case SDLK_s: down   = true; break;
            case SDLK_d: right  = true; break;
            case SDLK_a: left   = true; break;
            case SDLK_ESCAPE: running = false; break;
          }
          break;
        case SDL_KEYUP:
          switch(event.key.keysym.sym) {
            case SDLK_w: up     = false; break;
            case SDLK_s: down   = false; break;
            case SDLK_d: right  = false; break;
            case SDLK_a: left   = false; break;
          }
          break;
        case SDL_MOUSEMOTION:
          camera.rotate(event.motion.xrel, -event.motion.yrel);
          break;
        case SDL_MOUSEWHEEL:
          camera.zoom(-event.wheel.y);
          break;
        case SDL_QUIT:
          running = false;
          break;
      }

    glm::vec2 translation = glm::vec2(0.0f);
    if(up)    translation.y += 1.0f;
    if(down)  translation.y -= 1.0f;
    if(right) translation.x += 1.0f;
    if(left)  translation.x -= 1.0f;
    if(glm::length(translation) != 0.0f)
    {
      translation = glm::normalize(translation);
      translation *= dt;
      camera.translate(translation.x, translation.y);
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

    glm::mat4 view       = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    view       = camera.view();
    projection = camera.projection();

    for(size_t i=0; i<sizeof cubePositions / sizeof cubePositions[0]; ++i)
    {
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, cubePositions[i]);
      model = glm::rotate(model, glm::radians(i * 20.0f), glm::vec3(0.5f, 1.0f, 0.0f));
      if(i % 3 == 0)
        model = glm::rotate(model, (float)SDL_GetTicks() / 1000.0f * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));

      glm::mat4 transform = projection * view * model;
      glUniformMatrix4fv(glGetUniformLocation(program, "transform"), 1, GL_FALSE, glm::value_ptr(transform));

      mesh.draw();
    }

    SDL_GL_SwapWindow(window);
  }
}
