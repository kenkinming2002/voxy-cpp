#ifndef CHUNK_MANAGER_HPP
#define CHUNK_MANAGER_HPP

#include <chunk_defs.hpp>
#include <chunk_info.hpp>
#include <chunk_data.hpp>
#include <chunk_mesh.hpp>
#include <chunk_generator.hpp>

#include <camera.hpp>
#include <light.hpp>

#include <texture_array.hpp>

#include <gl.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <unordered_set>

#include <condition_variable>
#include <shared_mutex>

#include <thread>

class ChunkManager
{
public:
  ChunkManager(std::size_t seed);

public:
  void load(glm::ivec2 chunk_position);
  void load(glm::ivec2 center, int radius);

  void render(const Camera& camera, const Light& light) const;

public:
  std::optional<Block> get_block(glm::ivec3 position) const;
  bool set_block(glm::ivec3 position, Block block);

private:
  ChunkGenerator m_generator;
  std::unordered_map<glm::ivec2, std::pair<ChunkData, Mesh>> m_chunks;

private:
  std::vector<BlockData> m_block_datas;

private:
  gl::Program  m_program;
  TextureArray m_blocks_texture_array;
};

#endif // CHUNK_MANAGER_HPP
