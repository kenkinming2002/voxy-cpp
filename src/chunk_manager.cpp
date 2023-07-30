#include <chunk_manager.hpp>
#include <chunk_coords.hpp>

#include <camera.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <spdlog/spdlog.h>

static constexpr size_t LIGHTING_UPDATE_PER_FRAME = 20000;

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

void ChunkManager::update()
{
  lighting_update();
  for(auto& [chunk_position, chunk] : m_chunks)
    chunk.update(chunk_position, *this, m_block_datas);
}

void ChunkManager::render(const Camera& camera) const
{
  glm::mat4 view       = camera.view();
  glm::mat4 projection = camera.projection();
  glUseProgram(m_program);
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_blocks_texture_array.id());
    glUniform1i(glGetUniformLocation(m_program, "blocksTextureArray"), 0);
    for(const auto& [chunk_position, chunk] : m_chunks)
    {
      glm::mat4 model = glm::mat4(1.0f);
      glm::mat4 MVP   = projection * view * model;
      glUniformMatrix4fv(glGetUniformLocation(m_program, "MVP"),    1, GL_FALSE, glm::value_ptr(MVP));
      chunk.mesh->draw();
    }
  }
}

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

  auto [it, success] = m_chunks.emplace(chunk_position, Chunk());

  assert(success);
  Chunk& chunk = it->second;

  chunk.generate(chunk_position, m_generator);
  chunk.remash(chunk_position, *this, m_block_datas);

  // Lighting update on chunk load
  for(int lz=0; lz<CHUNK_HEIGHT; ++lz)
    for(int ly=0; ly<CHUNK_WIDTH; ++ly)
      for(int lx=0; lx<CHUNK_WIDTH; ++lx)
      {
        glm::ivec3 position = { lx, ly, lz };
        lighting_invalidate(local_to_global(position, chunk_position));
      }
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

std::optional<Block> ChunkManager::get_block(glm::ivec3 position) const
{
  // FIXME: Refactor me`
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
  // FIXME: Refactor me`
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

void ChunkManager::lighting_invalidate(glm::ivec3 position)
{
  m_pending_lighting_updates.insert(position);
}

void ChunkManager::lighting_update()
{
  for(unsigned i=0; i<LIGHTING_UPDATE_PER_FRAME; ++i)
  {
    if(m_pending_lighting_updates.empty())
      break;

    glm::ivec3 position = *m_pending_lighting_updates.begin();
    m_pending_lighting_updates.erase(m_pending_lighting_updates.begin());

    std::optional<Block> block = get_block(position);
    if(!block)
      continue;

    // FIXME: Refactor me`
    glm::ivec3 local_position = {
      modulo(position.x, CHUNK_WIDTH),
      modulo(position.y, CHUNK_WIDTH),
      position.z
    };

    glm::ivec2 chunk_position = {
      (position.x - local_position.x) / CHUNK_WIDTH,
      (position.y - local_position.y) / CHUNK_WIDTH,
    };
    Chunk& chunk = m_chunks.at(chunk_position);

    // 1: Skylight
    int sky_light_level = 15;
    for(int z=position.z+1; z<CHUNK_HEIGHT; ++z)
    {
      glm::ivec3 neighbour_position = { position.x, position.y, z };
      Block      neighbour_block    = chunk.get_block(global_to_local(neighbour_position, chunk_position)).value();
      if(neighbour_block.presence) // TODO: Opaqueness
      {
        sky_light_level = 0;
        break;
      }
    }

    // 2: TODO: Emitters

    // 3: Neighbours
    int neighbour_light_level = 0;
    for(glm::ivec3 direction : DIRECTIONS)
    {
      glm::ivec3 neighbour_position = position + direction;
      Block      neighbour_block    = get_block(neighbour_position).value_or(Block{.light_level = 15});
      neighbour_light_level = std::max<int>(neighbour_light_level, neighbour_block.light_level);
    }
    neighbour_light_level = std::max(neighbour_light_level-1, 0);

    int new_light_level = std::max(sky_light_level, neighbour_light_level);
    if(block->light_level != new_light_level)
    {
      block->light_level = new_light_level;
      set_block(position, *block);
      chunk.minor_invalidate_mesh();

      for(glm::ivec3 direction : DIRECTIONS)
      {
        glm::ivec3 neighbour_position = position + direction;
        m_pending_lighting_updates.insert(neighbour_position);
      }
    }
  }
  spdlog::info("Unresolved lighting updates count = {}", m_pending_lighting_updates.size());
}

