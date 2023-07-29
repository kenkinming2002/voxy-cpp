#include <chunk_manager.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <spdlog/spdlog.h>

ChunkManager::ChunkManager(std::size_t seed) :
  m_seed(seed),
  m_program(gl::compile_program("assets/chunk.vert", "assets/chunk.frag")),
  m_blocks_texture_array({
    "assets/stone.png",
    "assets/grass_bottom.png",
    "assets/grass_side.png",
    "assets/grass_top.png",
  })
{
  unsigned count = std::thread::hardware_concurrency();
  for(unsigned i=0; i<count; ++i)
    m_workers.emplace_back(std::bind(&ChunkManager::work, this, std::placeholders::_1));

  m_block_datas = {
    { .texture_indices = {0, 0, 0, 0, 0, 0} },
    { .texture_indices = {2, 2, 2, 2, 1, 3} },
  };
}

void ChunkManager::load(glm::ivec2 center, int radius)
{
  {
    std::lock_guard lk(m_mutex);
    for(int cy = center.y - radius; cy <= center.y + radius; ++cy)
      for(int cx = center.x - radius; cx <= center.x + radius; ++cx)
      {
        glm::ivec2 chunk_position(cx, cy);
        try_load_mesh(chunk_position);
      }
  }
  m_cv.notify_all();
}

void ChunkManager::render(const Camera& camera, const Light& light) const
{
  glm::mat4 view       = camera.view();
  glm::mat4 projection = camera.projection();

  glUseProgram(m_program);
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_blocks_texture_array.id());
    glUniform1i(glGetUniformLocation(m_program, "blocksTextureArray"), 0);

    glUniform3fv(glGetUniformLocation(m_program, "viewPos"),        1, glm::value_ptr(camera.transform.position));
    glUniform3fv(glGetUniformLocation(m_program, "light.pos"),      1, glm::value_ptr(light.position));
    glUniform3fv(glGetUniformLocation(m_program, "light.ambient"),  1, glm::value_ptr(light.ambient));
    glUniform3fv(glGetUniformLocation(m_program, "light.diffuse"),  1, glm::value_ptr(light.diffuse));

    std::shared_lock shared_lk(m_mutex);
    for(const auto& [chunk_position, chunk_mesh] : m_chunk_meshes)
    {
      glm::mat4 model  = glm::translate(glm::mat4(1.0f), glm::vec3( CHUNK_WIDTH * chunk_position.x, CHUNK_WIDTH * chunk_position.y, 0.0f));
      glm::mat4 normal = glm::transpose(glm::inverse(model));
      glm::mat4 MVP    = projection * view * model;

      glUniformMatrix4fv(glGetUniformLocation(m_program, "MVP"),    1, GL_FALSE, glm::value_ptr(MVP));
      glUniformMatrix4fv(glGetUniformLocation(m_program, "model"),  1, GL_FALSE, glm::value_ptr(model));
      glUniformMatrix4fv(glGetUniformLocation(m_program, "normal"), 1, GL_FALSE, glm::value_ptr(normal));

      chunk_mesh.draw();
    }
  }
}

bool ChunkManager::try_load_info(glm::ivec2 chunk_position)
{
  if(m_chunk_infos.contains(chunk_position))         return true;
  if(m_pending_chunk_infos.contains(chunk_position)) return false;
  if(m_loading_chunk_infos.contains(chunk_position)) return false;

  m_pending_chunk_infos.insert(chunk_position);
  return false;
}

bool ChunkManager::try_load_data(glm::ivec2 chunk_position)
{
  if(m_chunk_datas.contains(chunk_position))         return true;
  if(m_pending_chunk_datas.contains(chunk_position)) return false;
  if(m_loading_chunk_datas.contains(chunk_position)) return false;

  bool success = true;

  int        radius  = std::ceil(CAVE_WORM_SEGMENT_MAX * CAVE_WORM_STEP / CHUNK_WIDTH);
  glm::ivec2 corner1 = chunk_position - glm::ivec2(radius, radius);
  glm::ivec2 corner2 = chunk_position + glm::ivec2(radius, radius);
  for(int cy = corner1.y; cy <= corner2.y; ++cy)
    for(int cx = corner1.x; cx <= corner2.x; ++cx)
    {
      glm::ivec2 neighbour_chunk_position = glm::ivec2(cx, cy);
      if(!try_load_info(neighbour_chunk_position))
        success = false;
    }

  if(success) m_pending_chunk_datas.insert(chunk_position);
  return false;
}

bool ChunkManager::try_load_mesh(glm::ivec2 chunk_position)
{
  if(m_chunk_meshes.contains(chunk_position))         return true;
  if(m_pending_chunk_meshes.contains(chunk_position)) return false;
  if(m_loading_chunk_meshes.contains(chunk_position)) return false;

  bool success = try_load_data(chunk_position);

  if(success) m_pending_chunk_meshes.insert(chunk_position);
  return false;
}

void ChunkManager::work(std::stop_token stoken)
{
  std::unique_lock lk(m_mutex);
  for(;;)
  {
    m_cv.wait(lk, stoken, [this]() {
      return !m_pending_chunk_infos.empty()
          || !m_pending_chunk_datas.empty()
          || !m_pending_chunk_meshes.empty();
    });
    if(stoken.stop_requested())
      return;

    for(;;)
    {
      if(stoken.stop_requested())
        return;
      else if(!m_pending_chunk_meshes.empty())
      {
        glm::ivec2 chunk_position = *m_pending_chunk_meshes.begin();
        m_pending_chunk_meshes.erase(m_pending_chunk_meshes.begin());
        m_loading_chunk_meshes.insert(chunk_position);
        spdlog::info("Generating chunk mesh at {}, {}", chunk_position.x, chunk_position.y);

        lk.unlock();
        Mesh chunk_mesh = generate_chunk_mesh(chunk_position, *this);
        lk.lock();

        m_loading_chunk_datas.erase(chunk_position);
        m_chunk_meshes.emplace(chunk_position, std::move(chunk_mesh));
      }
      else if(!m_pending_chunk_datas.empty())
      {
        glm::ivec2 chunk_position = *m_pending_chunk_datas.begin();
        m_pending_chunk_datas.erase(m_pending_chunk_datas.begin());
        m_loading_chunk_datas.insert(chunk_position);
        spdlog::info("Generating chunk data at {}, {}", chunk_position.x, chunk_position.y);

        lk.unlock();
        ChunkData chunk_data = ChunkData::generate(chunk_position, *this);
        lk.lock();

        m_loading_chunk_datas.erase(chunk_position);
        m_chunk_datas.emplace(chunk_position, std::move(chunk_data));
      }
      else if(!m_pending_chunk_infos.empty())
      {
        glm::ivec2 chunk_position = *m_pending_chunk_infos.begin();
        m_pending_chunk_infos.erase(m_pending_chunk_infos.begin());
        m_loading_chunk_infos.insert(chunk_position);
        spdlog::info("Generating chunk info at {}, {}", chunk_position.x, chunk_position.y);

        lk.unlock();
        ChunkInfo chunk_info = ChunkInfo::generate(chunk_position, m_seed);
        lk.lock();

        m_loading_chunk_infos.erase(chunk_position);
        m_chunk_infos.emplace(chunk_position, std::move(chunk_info));
      }
      else
        break;
    }
  }
}

