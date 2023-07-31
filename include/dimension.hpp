#ifndef DIMENSION_HPP
#define DIMENSION_HPP

#include <chunk.hpp>

#include <system/chunk_generator.hpp>
#include <system/chunk_mesher.hpp>
#include <system/light.hpp>

#include <texture_array.hpp>

#include <gl.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <unordered_set>

struct Camera;
class Dimension
{
public:
  Dimension(std::size_t seed);

public:
  void update();
  void render(const Camera& camera) const;

public:
  Chunk& get_chunk(glm::ivec2 chunk_index) { return m_chunks[chunk_index]; }
  const std::vector<BlockData>& block_datas() const { return m_block_datas; }
  std::unordered_set<glm::ivec3>& pending_lighting_updates() { return m_pending_lighting_updates; }

public:
  void load(glm::ivec2 chunk_index);
  void load(glm::ivec2 center, int radius);

public:
  std::optional<Block> get_block(glm::ivec3 position) const;
  bool set_block(glm::ivec3 position, Block block);

public:
  void major_invalidate_mesh(glm::ivec3 position);
  void minor_invalidate_mesh(glm::ivec3 position);

private:
  void lighting_invalidate(glm::ivec3 position);

private:
  std::unordered_map<glm::ivec2, Chunk> m_chunks;

private:
  std::unique_ptr<ChunkGeneratorSystem> m_chunk_generator_system;
  std::unique_ptr<ChunkMesherSystem>    m_chunk_mesher_system;
  std::unique_ptr<LightSystem>          m_light_system;

private:
  std::vector<BlockData> m_block_datas;

private:
  gl::Program  m_program;
  TextureArray m_blocks_texture_array;

private:
  std::unordered_set<glm::ivec3> m_pending_lighting_updates;
};

#endif // DIMENSION_HPP
