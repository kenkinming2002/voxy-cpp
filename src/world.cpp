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

World::World() :
  light_program(gl::compile_program("assets/light.vert", "assets/light.frag")),
  chunk_program(gl::compile_program("assets/chunk.vert", "assets/chunk.frag")),
  light_mesh({}, {})
{
  light.pos      = glm::vec3(0.0f, 0.0f, 30.0f);
  light.ambient  = glm::vec3(0.2f, 0.2f, 0.2f);
  light.diffuse  = glm::vec3(0.5f, 0.5f, 0.5f);
  light.specular = glm::vec3(1.0f, 1.0f, 1.0f);

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

  light_mesh = Mesh(indices, vertices);

  chunk_material.ambient   = glm::vec3(1.0f, 0.5f, 0.31f);
  chunk_material.diffuse   = glm::vec3(1.0f, 0.5f, 0.31f);
  chunk_material.specular  = glm::vec3(0.5f, 0.5f, 0.5f);
  chunk_material.shininess = 32.0f;
}

void World::generate_chunk(glm::ivec2 cpos)
{
  if(chunks.contains(cpos))
    return;

  int stone_heights[Layer::WIDTH][Layer::WIDTH];
  for(int cy=0; cy<Layer::WIDTH; ++cy)
    for(int cx=0; cx<Layer::WIDTH; ++cx)
    {
      glm::vec2 pos = glm::vec2(Layer::WIDTH * cpos) + glm::vec2(cx, cy);
      stone_heights[cy][cx] = 0.0f;
      stone_heights[cy][cx] += perlin(pos, 0.1f  / Layer::WIDTH, 1.0f);
      stone_heights[cy][cx] += perlin(pos, 0.25f / Layer::WIDTH, 5.0f);
      stone_heights[cy][cx] += perlin(pos, 0.5f  / Layer::WIDTH, 10.0f);
      stone_heights[cy][cx] += perlin(pos, 1.0f  / Layer::WIDTH, 20.0f);
    }

  int grass_heights[Layer::WIDTH][Layer::WIDTH];
  for(int cy=0; cy<Layer::WIDTH; ++cy)
    for(int cx=0; cx<Layer::WIDTH; ++cx)
    {
      glm::vec2 pos = glm::vec2(Layer::WIDTH * cpos) + glm::vec2(cx, cy);
      grass_heights[cy][cx] = 0.0f;
      grass_heights[cy][cx] += perlin(pos, 0.5f  / Layer::WIDTH, 2.0f);
      grass_heights[cy][cx] += perlin(pos, 1.0f  / Layer::WIDTH, 5.0f);
    }

  int max_height = 0;
  for(int cy=0; cy<Layer::WIDTH; ++cy)
    for(int cx=0; cx<Layer::WIDTH; ++cx)
      if(max_height < stone_heights[cy][cx]+grass_heights[cy][cx])
        max_height = stone_heights[cy][cx]+grass_heights[cy][cx];

  Chunk chunk;
  for(int cz=0; cz<max_height; ++cz)
  {
    Layer layer;
    for(int cy=0; cy<Layer::WIDTH; ++cy)
      for(int cx=0; cx<Layer::WIDTH; ++cx)
        if(cz <= stone_heights[cy][cx])
          layer.blocks[cy][cx] = Block {
            .presence = true,
            .color    = glm::vec3(0.2, 1.0, 0.2),
          };
        else if(cz <= stone_heights[cy][cx] + grass_heights[cy][cx])
          layer.blocks[cy][cx] = Block {
            .presence = true,
            .color    = glm::vec3(0.7, 0.7, 0.7),
          };
        else
          layer.blocks[cy][cx] = Block {
            .presence = false,
          };

    chunk.layers.push_back(layer);
  }

  chunks.insert({cpos, std::move(chunk)});
}

void World::generate_chunk_mesh(glm::ivec2 cpos)
{
  if(chunk_meshes.contains(cpos))
    return;

  Chunk& chunk = chunks.at(cpos);

  std::vector<uint32_t> indices;
  std::vector<Vertex>   vertices;
  for(int z=0; z<chunk.layers.size(); ++z)
    for(int y=0; y<Layer::WIDTH; ++y)
      for(int x=0; x<Layer::WIDTH; ++x)
      {
        const Block& block = chunk.layers[z].blocks[y][x];
        if(block.presence)
        {
          uint32_t index_base = vertices.size();
          for(uint32_t cube_index : CUBE_INDICES)
            indices.push_back(index_base + cube_index);

          glm::vec3 position_base = glm::vec3(x, y, z);
          for(auto [cube_position, cube_normal] : CUBE_VERTICES)
            vertices.push_back(Vertex{
                .pos    = position_base + cube_position,
                .normal = cube_normal,
                .color  = block.color,
            });
        }
      }

  Mesh mesh(indices, vertices);
  chunk_meshes.insert({cpos, std::move(mesh)});
}

void World::unload(glm::vec2 center, float radius)
{
  glm::ivec2 ccenter = center / (float)Layer::WIDTH;
  int        cradius = radius / Layer::WIDTH;

  for(auto it = chunk_meshes.begin(); it != chunk_meshes.end();)
  {
    const auto& [cpos, chunk_mesh] = *it;
    glm::ivec2 coff = cpos - ccenter;
    if(coff.x * coff.x + coff.y * coff.y > cradius)
      it = chunk_meshes.erase(it);
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
        generate_chunk(cpos);
        generate_chunk_mesh(cpos);
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

    for(const auto& [cpos, chunk_mesh] : chunk_meshes)
    {
      glm::mat4 model  = glm::translate(glm::mat4(1.0f), glm::vec3( Layer::WIDTH * cpos.x, Layer::WIDTH * cpos.y, 0.0f));
      glm::mat4 normal = glm::transpose(glm::inverse(model));
      glm::mat4 MVP    = projection * view * model;

      glUniformMatrix4fv(glGetUniformLocation(chunk_program, "MVP"),    1, GL_FALSE, glm::value_ptr(MVP));
      glUniformMatrix4fv(glGetUniformLocation(chunk_program, "model"),  1, GL_FALSE, glm::value_ptr(model));
      glUniformMatrix4fv(glGetUniformLocation(chunk_program, "normal"), 1, GL_FALSE, glm::value_ptr(normal));

      chunk_mesh.draw();
    }
  }
}
