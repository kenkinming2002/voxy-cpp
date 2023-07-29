#ifndef CHUNK_MANAGER_HPP
#define CHUNK_MANAGER_HPP

#include <chunk_defs.hpp>
#include <chunk_info.hpp>
#include <chunk_data.hpp>
#include <chunk_mesh.hpp>

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
  void load(glm::ivec2 center, int radius);
  void render(const Camera& camera, const Light& light) const;

public:
  auto& mutex()        const { return m_mutex; }
  auto& chunk_infos()  const { return m_chunk_infos; }
  auto& chunk_datas()  const { return m_chunk_datas; }
  auto& chunk_meshes() const { return m_chunk_meshes; }

public:
  auto& block_datas() const { return m_block_datas; }

private:
  // Try to load info/data/mesh
  //
  // @precondition m_mutex is held
  // @return       true if info/data/mesh has already been loaded, false otherwise
  bool try_load_info(glm::ivec2 chunk_position);
  bool try_load_data(glm::ivec2 chunk_position);
  bool try_load_mesh(glm::ivec2 chunk_position);

  void work(std::stop_token stoken);

private:
  std::size_t m_seed;

private:
  std::shared_mutex           mutable m_mutex;
  std::condition_variable_any         m_cv;

private:
  std::unordered_set<glm::ivec2> m_pending_chunk_infos;
  std::unordered_set<glm::ivec2> m_pending_chunk_datas;
  std::unordered_set<glm::ivec2> m_pending_chunk_meshes;

  std::unordered_set<glm::ivec2> m_loading_chunk_infos;
  std::unordered_set<glm::ivec2> m_loading_chunk_datas;
  std::unordered_set<glm::ivec2> m_loading_chunk_meshes;

  std::unordered_map<glm::ivec2, ChunkInfo> m_chunk_infos;
  std::unordered_map<glm::ivec2, ChunkData> m_chunk_datas;
  std::unordered_map<glm::ivec2, Mesh>      m_chunk_meshes;

private:
  std::vector<BlockData> m_block_datas;

private:
  gl::Program  m_program;
  TextureArray m_blocks_texture_array;

private:
  std::vector<std::jthread> m_workers;
};

#endif // CHUNK_MANAGER_HPP
