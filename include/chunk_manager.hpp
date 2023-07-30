#ifndef CHUNK_MANAGER_HPP
#define CHUNK_MANAGER_HPP

#include <chunk.hpp>
#include <chunk_generator.hpp>

#include <texture_array.hpp>

#include <gl.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>

struct Camera;
class ChunkManager
{
public:
  ChunkManager(std::size_t seed);

public:
  void update();
  void render(const Camera& camera) const;

public:
  void load(glm::ivec2 chunk_position);
  void load(glm::ivec2 center, int radius);

public:
  std::optional<Block> get_block(glm::ivec3 position) const;
  bool set_block(glm::ivec3 position, Block block);

private:
  void lighting_invalidate(glm::ivec3 position);
  void lighting_update();

private:
  ChunkGenerator                        m_generator;
  std::unordered_map<glm::ivec2, Chunk> m_chunks;

private:
  std::vector<BlockData> m_block_datas;

private:
  gl::Program  m_program;
  TextureArray m_blocks_texture_array;

private:
  std::unordered_set<glm::ivec3> m_pending_lighting_updates;
};

#endif // CHUNK_MANAGER_HPP
