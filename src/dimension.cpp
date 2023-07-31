#include <dimension.hpp>

#include <chunk_coords.hpp>

#include <camera.hpp>

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

Dimension::Dimension(std::size_t seed) :
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

void Dimension::update()
{
}

void Dimension::render(const Camera& camera) const
{
  glm::mat4 view       = camera.view();
  glm::mat4 projection = camera.projection();
  glUseProgram(m_program);
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_blocks_texture_array.id());
    glUniform1i(glGetUniformLocation(m_program, "blocksTextureArray"), 0);
    for(const auto& [chunk_index, chunk] : m_chunks)
    {
      glm::mat4 model = glm::mat4(1.0f);
      glm::mat4 MVP   = projection * view * model;
      glUniformMatrix4fv(glGetUniformLocation(m_program, "MVP"),    1, GL_FALSE, glm::value_ptr(MVP));
      if(chunk.mesh)
        chunk.mesh->draw();
    }
  }
}

std::optional<Block> Dimension::get_block(glm::ivec3 position) const
{
  // FIXME: Refactor me`
  glm::ivec3 local_position = {
    modulo(position.x, CHUNK_WIDTH),
    modulo(position.y, CHUNK_WIDTH),
    position.z
  };

  glm::ivec2 chunk_index = {
    (position.x - local_position.x) / CHUNK_WIDTH,
    (position.y - local_position.y) / CHUNK_WIDTH,
  };

  auto it = m_chunks.find(chunk_index);
  if(it == m_chunks.end())
    return std::nullopt;

  return it->second.get_block(local_position);
}

bool Dimension::set_block(glm::ivec3 position, Block block)
{
  // FIXME: Refactor me`
  glm::ivec3 local_position = {
    modulo(position.x, CHUNK_WIDTH),
    modulo(position.y, CHUNK_WIDTH),
    position.z
  };

  glm::ivec2 chunk_index = {
    (position.x - local_position.x) / CHUNK_WIDTH,
    (position.y - local_position.y) / CHUNK_WIDTH,
  };

  auto it = m_chunks.find(chunk_index);
  if(it == m_chunks.end())
    return false;

  return it->second.set_block(local_position, block);
}

void Dimension::major_invalidate_mesh(glm::ivec3 position)
{
  glm::ivec3 local_position = {
    modulo(position.x, CHUNK_WIDTH),
    modulo(position.y, CHUNK_WIDTH),
    position.z
  };
  glm::ivec2 chunk_index = {
    (position.x - local_position.x) / CHUNK_WIDTH,
    (position.y - local_position.y) / CHUNK_WIDTH,
  };
  get_chunk(chunk_index).major_invalidate_mesh();
}

void Dimension::minor_invalidate_mesh(glm::ivec3 position)
{
  glm::ivec3 local_position = {
    modulo(position.x, CHUNK_WIDTH),
    modulo(position.y, CHUNK_WIDTH),
    position.z
  };
  glm::ivec2 chunk_index = {
    (position.x - local_position.x) / CHUNK_WIDTH,
    (position.y - local_position.y) / CHUNK_WIDTH,
  };
  get_chunk(chunk_index).minor_invalidate_mesh();
}

void Dimension::lighting_invalidate(glm::ivec3 position)
{
  m_pending_lighting_updates.insert(position);
}

