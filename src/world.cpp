#include <world.hpp>

#include <perlin.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include <math.h>

static constexpr uint32_t CUBE_INDICES[] = {
  0,  1,  2,  2,  1,  3,
  6,  5,  4,  7,  5,  6,
  10,  9,  8, 11, 9,  10,
  12, 13, 14, 14, 13, 15,
  16, 17, 18, 18, 17, 19,
  22, 21, 20, 23, 21, 22,
};

static constexpr std::pair<glm::vec3, glm::vec3> CUBE_VERTICES[] = {
  {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
  {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
  {{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
  {{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},

  {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
  {{0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
  {{1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
  {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},

  {{0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
  {{0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
  {{1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
  {{1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},

  {{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
  {{0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
  {{1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
  {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},

  {{0.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
  {{0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},
  {{0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
  {{0.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},

  {{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
  {{1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
  {{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
  {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
};

struct Vertex
{
  glm::vec3 pos;
  glm::vec3 normal;
  glm::vec3 color;
};

static Mesh generate_light_mesh(Light light)
{
  std::vector<uint32_t> indices;
  std::vector<Vertex>   vertices;

  for(uint32_t cube_index : CUBE_INDICES)
    indices.push_back(cube_index);

  for(auto [cube_position, cube_normal] : CUBE_VERTICES)
    vertices.push_back(Vertex{
        .pos    = cube_position,
        .normal = cube_normal,
        .color  = glm::vec3(1.0, 1.0, 1.0),
    });

  return Mesh(indices, VertexLayout{
    .stride = sizeof(Vertex),
    .attributes = {
      { .offset = offsetof(Vertex, pos),    .count = 3, },
      { .offset = offsetof(Vertex, normal), .count = 3, },
      { .offset = offsetof(Vertex, color),  .count = 3, },
    },
  }, std::as_bytes(std::span(vertices)));
}

World::World() :
  camera{
    .position = glm::vec3(-5.0f, -5.0f,  50.0f),
    .aspect = 1024.0f / 720.0f,
    .yaw    = 45.0f,
    .pitch  = -45.0f,
    .fov    = 45.0f,
  },
  light_program(gl::compile_program("assets/light.vert", "assets/light.frag")),
  chunk_program(gl::compile_program("assets/chunk.vert", "assets/chunk.frag")),
  light{
    .pos      = glm::vec3(0.0f, 0.0f, 30.0f),
    .ambient  = glm::vec3(0.2f, 0.2f, 0.2f),
    .diffuse  = glm::vec3(0.5f, 0.5f, 0.5f),
    .specular = glm::vec3(1.0f, 1.0f, 1.0f),
  },
  light_mesh(generate_light_mesh(light)),
  chunk_material{
    .ambient   = glm::vec3(1.0f, 0.5f, 0.31f),
    .diffuse   = glm::vec3(1.0f, 0.5f, 0.31f),
    .specular  = glm::vec3(0.5f, 0.5f, 0.5f),
    .shininess = 32.0f,
  }
{}

void World::unload(glm::vec2 center, float radius)
{
  glm::ivec2 ccenter = center / (float)Layer::WIDTH;
  int        cradius = radius / Layer::WIDTH;

  for(auto it = chunks.begin(); it != chunks.end();)
  {
    const auto& [cpos, chunk_mesh] = *it;
    glm::ivec2 coff = cpos - ccenter;
    if(coff.x * coff.x + coff.y * coff.y > cradius)
      it = chunks.erase(it);
    else
      ++it;
  }
}

void World::load(glm::vec2 center, float radius)
{
  glm::ivec2 ccenter = center / (float)Layer::WIDTH;
  int        cradius = radius / Layer::WIDTH;

  for(int yoff = -cradius; yoff <= cradius; ++yoff)
    for(int xoff = -cradius; xoff <= cradius; ++xoff)
    {
      glm::ivec2 coff(xoff, yoff);
      if(xoff * xoff + yoff * yoff < cradius)
      {
        glm::ivec2 cpos = ccenter + coff;
        if(!chunks.contains(cpos))
          chunks.emplace(std::piecewise_construct,
            std::forward_as_tuple(cpos),
            std::forward_as_tuple(cpos)
          );
      }
    }
}

void World::handle_event(SDL_Event event)
{
  switch(event.type) {
    case SDL_MOUSEMOTION:
      camera.rotate(-event.motion.xrel, -event.motion.yrel);
      break;
    case SDL_MOUSEWHEEL:
      camera.zoom(-event.wheel.y);
      break;
  }
}

void World::update(float dt)
{
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

  unload(camera.position, 300.0f);
  load  (camera.position, 300.0f);

  light.pos.x += 5.0f * dt;
}

void World::render()
{
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 view       = camera.view();
  glm::mat4 projection = camera.projection();

  // 1: Light
  glUseProgram(light_program);
  {
    glm::mat4 model  = glm::translate(glm::mat4(1.0f), light.pos);
    glm::mat4 MVP    = projection * view * model;
    glUniformMatrix4fv(glGetUniformLocation(light_program, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
    light_mesh.draw();
  }

  // 2: Chunks
  glUseProgram(chunk_program);
  {
    glUniform3fv(glGetUniformLocation(chunk_program, "viewPos"),        1, glm::value_ptr(camera.position));
    glUniform3fv(glGetUniformLocation(chunk_program, "light.pos"),      1, glm::value_ptr(light.pos));
    glUniform3fv(glGetUniformLocation(chunk_program, "light.ambient"),  1, glm::value_ptr(light.ambient));
    glUniform3fv(glGetUniformLocation(chunk_program, "light.diffuse"),  1, glm::value_ptr(light.ambient));
    glUniform3fv(glGetUniformLocation(chunk_program, "light.specular"), 1, glm::value_ptr(light.specular));

    glUniform3fv(glGetUniformLocation(chunk_program, "material.ambient"),   1, glm::value_ptr(chunk_material.ambient));
    glUniform3fv(glGetUniformLocation(chunk_program, "material.diffuse"),   1, glm::value_ptr(chunk_material.diffuse));
    glUniform3fv(glGetUniformLocation(chunk_program, "material.specular"),  1, glm::value_ptr(chunk_material.specular));
    glUniform1f (glGetUniformLocation(chunk_program, "material.shininess"), chunk_material.shininess);

    for(const auto& [cpos, chunk] : chunks)
    {
      glm::mat4 model  = glm::translate(glm::mat4(1.0f), glm::vec3( Layer::WIDTH * cpos.x, Layer::WIDTH * cpos.y, 0.0f));
      glm::mat4 normal = glm::transpose(glm::inverse(model));
      glm::mat4 MVP    = projection * view * model;

      glUniformMatrix4fv(glGetUniformLocation(chunk_program, "MVP"),    1, GL_FALSE, glm::value_ptr(MVP));
      glUniformMatrix4fv(glGetUniformLocation(chunk_program, "model"),  1, GL_FALSE, glm::value_ptr(model));
      glUniformMatrix4fv(glGetUniformLocation(chunk_program, "normal"), 1, GL_FALSE, glm::value_ptr(normal));

      chunk.layers_mesh.draw();
    }
  }
}
