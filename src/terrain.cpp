#include <terrain.hpp>

#include <glm/gtc/type_ptr.hpp>

Terrain::Terrain() :
  program(gl::compile_program("assets/chunk.vert", "assets/chunk.frag")),
  chunks_mutex(),
  chunks(),
  load_cv(),
  load_mutex(),
  center(),
  radius(500.0f),
  worker([this](std::stop_token stoken){
    std::unique_lock load_lk(load_mutex);
    for(;;)
    {
      glm::vec2 center = this->center;
      float     radius = this->radius;

      load_lk.unlock();

      std::vector<glm::ivec2> positions_chunk;

      // 1: Get a list of chunks that need to be loaded
      glm::ivec2 center_chunk = center / (float)Layer::WIDTH;
      int        radius_chunk = radius / Layer::WIDTH;
      for(int yoffset_chunk = -radius_chunk; yoffset_chunk <= radius_chunk; ++yoffset_chunk)
        for(int xoffset_chunk = -radius_chunk; xoffset_chunk <= radius_chunk; ++xoffset_chunk)
        {
          glm::ivec2 offset_chunk   = glm::ivec2(xoffset_chunk, yoffset_chunk);
          glm::ivec2 position_chunk = center_chunk + offset_chunk;

          if(offset_chunk.x * offset_chunk.x + offset_chunk.y * offset_chunk.y > radius_chunk)
            continue;

          std::lock_guard chunks_lk(chunks_mutex);
          if(chunks.contains(position_chunk))
            continue;

          positions_chunk.push_back(position_chunk);
        }

      // 2: Load them one by one
      for(glm::ivec2 position_chunk : positions_chunk)
      {
        Chunk chunk(position_chunk);

        std::unique_lock chunks_lk(chunks_mutex);
        chunks.emplace(position_chunk, std::move(chunk));
        chunks_lk.unlock();

        if(stoken.stop_requested())
          return;
      }

      load_lk.lock();

      if(!load_cv.wait(load_lk, stoken, [&]() { return center != this->center || radius != this->radius; }))
        return;
    }
  })
{}

void Terrain::load(glm::vec2 center, float radius)
{
  bool changed = false;
  std::unique_lock load_lk(load_mutex);
  if(this->center != center)
  {
    this->center = center;
    changed = true;
  }
  if(this->radius != radius)
  {
    this->radius = radius;
    changed = true;
  }
  load_lk.unlock();
  if(changed)
    load_cv.notify_one();
}

void Terrain::update(float dt)
{
  // Nothing to do for now
}

void Terrain::render(const Camera& camera, const Lights& lights)
{
  glm::mat4 view       = camera.view();
  glm::mat4 projection = camera.projection();

  glUseProgram(program);
  {
    glUniform3fv(glGetUniformLocation(program, "viewPos"),        1, glm::value_ptr(camera.position));
    glUniform3fv(glGetUniformLocation(program, "light.pos"),      1, glm::value_ptr(lights.light.pos));
    glUniform3fv(glGetUniformLocation(program, "light.ambient"),  1, glm::value_ptr(lights.light.ambient));
    glUniform3fv(glGetUniformLocation(program, "light.diffuse"),  1, glm::value_ptr(lights.light.diffuse));

    std::lock_guard chunks_lk(chunks_mutex);
    for(const auto& [cpos, chunk] : chunks)
    {
      glm::mat4 model  = glm::translate(glm::mat4(1.0f), glm::vec3( Layer::WIDTH * cpos.x, Layer::WIDTH * cpos.y, 0.0f));
      glm::mat4 normal = glm::transpose(glm::inverse(model));
      glm::mat4 MVP    = projection * view * model;

      glUniformMatrix4fv(glGetUniformLocation(program, "MVP"),    1, GL_FALSE, glm::value_ptr(MVP));
      glUniformMatrix4fv(glGetUniformLocation(program, "model"),  1, GL_FALSE, glm::value_ptr(model));
      glUniformMatrix4fv(glGetUniformLocation(program, "normal"), 1, GL_FALSE, glm::value_ptr(normal));

      chunk.layers_mesh.draw();
    }
  }
}

