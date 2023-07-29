#include <chunk_manager.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <spdlog/spdlog.h>

static int modulo(int a, int b)
{
  int value = a % b;
  if(value < 0)
    value += b;

  assert(0 <= value);
  assert(value < b);
  return value;
}

ChunkManager::ChunkManager(std::size_t seed) :
  m_generator(seed),
  m_block_datas{
    { .texture_indices = {0, 0, 0, 0, 0, 0} },
    { .texture_indices = {2, 2, 2, 2, 1, 3} },
  },
  m_program(gl::compile_program("assets/chunk.vert", "assets/chunk.frag")),
  m_blocks_texture_array({
    "assets/stone.png",
    "assets/grass_bottom.png",
    "assets/grass_side.png",
    "assets/grass_top.png",
  })
{}

void ChunkManager::load(glm::ivec2 chunk_position)
{
  if(m_chunks.contains(chunk_position))
    return;

  int        radius  = std::ceil(CAVE_WORM_SEGMENT_MAX * CAVE_WORM_STEP / CHUNK_WIDTH);
  glm::ivec2 corner1 = chunk_position - glm::ivec2(radius, radius);
  glm::ivec2 corner2 = chunk_position + glm::ivec2(radius, radius);
  for(int cy = corner1.y; cy <= corner2.y; ++cy)
    for(int cx = corner1.x; cx <= corner2.x; ++cx)
    {
      glm::ivec2 neighbour_chunk_position = glm::ivec2(cx, cy);
      if(!m_generator.try_get_chunk_info(neighbour_chunk_position))
        return;
    }

  Chunk chunk;
  chunk.generate(chunk_position, m_generator);
  chunk.remash(m_block_datas);
  m_chunks.emplace(chunk_position, std::move(chunk));
}

void ChunkManager::load(glm::ivec2 center, int radius)
{
  for(int cy = center.y - radius; cy <= center.y + radius; ++cy)
    for(int cx = center.x - radius; cx <= center.x + radius; ++cx)
    {
      glm::ivec2 chunk_position(cx, cy);
      load(chunk_position);
    }
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

    for(const auto& [chunk_position, chunk] : m_chunks)
    {
      const auto& [chunk_data, chunk_mesh] = chunk;

      glm::mat4 model  = glm::translate(glm::mat4(1.0f), glm::vec3( CHUNK_WIDTH * chunk_position.x, CHUNK_WIDTH * chunk_position.y, 0.0f));
      glm::mat4 normal = glm::transpose(glm::inverse(model));
      glm::mat4 MVP    = projection * view * model;

      glUniformMatrix4fv(glGetUniformLocation(m_program, "MVP"),    1, GL_FALSE, glm::value_ptr(MVP));
      glUniformMatrix4fv(glGetUniformLocation(m_program, "model"),  1, GL_FALSE, glm::value_ptr(model));
      glUniformMatrix4fv(glGetUniformLocation(m_program, "normal"), 1, GL_FALSE, glm::value_ptr(normal));

      chunk_mesh->draw();
    }
  }
}

std::optional<Block> ChunkManager::get_block(glm::ivec3 position) const
{
  glm::ivec3 local_position = {
    modulo(position.x, CHUNK_WIDTH),
    modulo(position.y, CHUNK_WIDTH),
    position.z
  };

  glm::ivec2 chunk_position = {
    (position.x - local_position.x) / CHUNK_WIDTH,
    (position.y - local_position.y) / CHUNK_WIDTH,
  };

  auto it = m_chunks.find(chunk_position);
  if(it == m_chunks.end())
    return std::nullopt;

  return it->second.get_block(local_position);
}

bool ChunkManager::set_block(glm::ivec3 position, Block block)
{
  glm::ivec3 local_position = {
    modulo(position.x, CHUNK_WIDTH),
    modulo(position.y, CHUNK_WIDTH),
    position.z
  };

  glm::ivec2 chunk_position = {
    (position.x - local_position.x) / CHUNK_WIDTH,
    (position.y - local_position.y) / CHUNK_WIDTH,
  };

  auto it = m_chunks.find(chunk_position);
  if(it == m_chunks.end())
    return false;

  return it->second.set_block(local_position, block);
}


