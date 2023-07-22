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

#include <iostream>
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

static const uint32_t indices[] = {
  0, 1, 2, 3, 4, 5,
  6, 7, 8, 9, 10, 11,
  12, 13, 14, 15, 16, 17,
  18, 19, 20, 21, 22, 23,
  24, 25, 26, 27, 28, 29,
  30, 31, 32, 33, 34, 35,
};

struct Chunk
{
  static constexpr size_t WIDTH = 16;
  uint16_t blocks[WIDTH][WIDTH][WIDTH];

  Mesh generate_mesh()
  {
    std::vector<uint32_t> indices;
    std::vector<Vertex>   vertices;
    for(size_t z=0; z<WIDTH; ++z)
      for(size_t y=0; y<WIDTH; ++y)
        for(size_t x=0; x<WIDTH; ++x)
          if(blocks[z][y][x])
          {
            static constexpr uint32_t cube_indices[] = {
              0,  1,  2,  2,  1,  3,
              6,  5,  4,  7,  5,  6,
              10,  9,  8, 11, 9,  10,
              12, 13, 14, 14, 13, 15,
              16, 17, 18, 18, 17, 19,
              22, 21, 20, 23, 21, 22,
            };

            static constexpr Vertex cube_vertices[] = {
              {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 1.0f}},
              {{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}},
              {{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 1.0f}},
              {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}},

              {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
              {{0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
              {{1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
              {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},

              {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
              {{0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
              {{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
              {{1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},

              {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
              {{0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
              {{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
              {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},

              {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
              {{0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
              {{0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
              {{0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},

              {{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
              {{1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
              {{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
              {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
            };

            uint32_t index_base = vertices.size();
            for(uint32_t cube_index : cube_indices)
            {
              cube_index += index_base;
              indices.push_back(cube_index);
            }

            glm::vec3 pos_base = glm::vec3(x, y, z);
            for(Vertex cube_vertex : cube_vertices)
            {
              cube_vertex.pos += pos_base;
              vertices.push_back(cube_vertex);
            }
          }

    return Mesh(indices, vertices);
  }
};

int main()
{
  sdl2::Context sdl2_context;
  sdl2::Window window("voxy", 800, 600);
  SDL_SetRelativeMouseMode(SDL_TRUE);

  if(!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    throw std::runtime_error("Failed to load OpenGL functions with GLAD");

  gl::init_debug();

  Mesh mesh(indices, vertices);

  Chunk chunk;
  for(size_t z=0; z<Chunk::WIDTH; ++z)
    for(size_t y=0; y<Chunk::WIDTH; ++y)
      for(size_t x=0; x<Chunk::WIDTH; ++x)
        chunk.blocks[y][x][z] = (x+y+z+1) % 2;

  Mesh chunk_mesh = chunk.generate_mesh();

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
          camera.rotate(event.motion.xrel, -event.motion.yrel);
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

    glm::mat4 view       = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    view       = camera.view();
    projection = camera.projection();

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 transform = projection * view * model;
    glUniformMatrix4fv(glGetUniformLocation(program, "transform"), 1, GL_FALSE, glm::value_ptr(transform));
    chunk_mesh.draw();

    SDL_GL_SwapWindow(window);
  }
}
