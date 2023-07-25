#include <terrain.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>

Terrain::Terrain() :
  m_program(gl::compile_program("assets/chunk.vert", "assets/chunk.frag")),
  m_mutex(),
  m_cv(),
  m_states(),
  m_workers()
{
  unsigned count = std::thread::hardware_concurrency();
  for(unsigned i=0; i<count; ++i)
    m_workers.emplace_back(std::bind(&Terrain::loader, this, std::placeholders::_1));
}

void Terrain::load(glm::vec2 center, float radius)
{
  std::unique_lock lk(m_mutex);

  glm::ivec2 center_chunk = center / (float)Blocks::WIDTH;
  int        radius_chunk = radius / Blocks::WIDTH;

  bool any = false;
  for(int yoffset_chunk = -radius_chunk; yoffset_chunk <= radius_chunk; ++yoffset_chunk)
    for(int xoffset_chunk = -radius_chunk; xoffset_chunk <= radius_chunk; ++xoffset_chunk)
    {
      glm::ivec2 offset_chunk   = glm::ivec2(xoffset_chunk, yoffset_chunk);
      glm::ivec2 position_chunk = center_chunk + offset_chunk;
      if(offset_chunk.x * offset_chunk.x + offset_chunk.y * offset_chunk.y > radius_chunk)
        continue;

      auto [it, success] = m_states.emplace(position_chunk, Pending{});
      if(success)
        any = true;
    }

  if(any)
    m_cv.notify_all();
}

void Terrain::update(float dt)
{
  // Nothing to do for now
}

void Terrain::render(const Camera& camera, const Lights& lights)
{
  glm::mat4 view       = camera.view();
  glm::mat4 projection = camera.projection();

  glUseProgram(m_program);
  {
    glUniform3fv(glGetUniformLocation(m_program, "viewPos"),        1, glm::value_ptr(camera.position));
    glUniform3fv(glGetUniformLocation(m_program, "light.pos"),      1, glm::value_ptr(lights.light.pos));
    glUniform3fv(glGetUniformLocation(m_program, "light.ambient"),  1, glm::value_ptr(lights.light.ambient));
    glUniform3fv(glGetUniformLocation(m_program, "light.diffuse"),  1, glm::value_ptr(lights.light.diffuse));

    std::lock_guard guard(m_mutex);
    for(const auto& [cpos, state] : m_states)
      if(std::holds_alternative<Chunk>(state))
      {
        const Chunk& chunk = std::get<Chunk>(state);

        glm::mat4 model  = glm::translate(glm::mat4(1.0f), glm::vec3( Blocks::WIDTH * cpos.x, Blocks::WIDTH * cpos.y, 0.0f));
        glm::mat4 normal = glm::transpose(glm::inverse(model));
        glm::mat4 MVP    = projection * view * model;

        glUniformMatrix4fv(glGetUniformLocation(m_program, "MVP"),    1, GL_FALSE, glm::value_ptr(MVP));
        glUniformMatrix4fv(glGetUniformLocation(m_program, "model"),  1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(m_program, "normal"), 1, GL_FALSE, glm::value_ptr(normal));

        chunk.blocks_mesh.draw();
      }
  }
}

void Terrain::loader(std::stop_token stoken)
{
  std::unique_lock lk(m_mutex);
  for(;;)
  {
    glm::ivec2 cpos;
    m_cv.wait(lk, stoken, [&]()
    {
      auto it = std::find_if(m_states.begin(), m_states.end(), [](auto& value)
      {
        auto& [_, state ] = value;
        return std::holds_alternative<Pending>(state);
      });

      if(it != m_states.end())
      {
        cpos = it->first;
        return true;
      }
      else
        return false;
    });

    if(stoken.stop_requested())
      return;

    lk.unlock();
    Chunk chunk(cpos);
    lk.lock();

    m_states.insert_or_assign(cpos, std::move(chunk));
  }
}

