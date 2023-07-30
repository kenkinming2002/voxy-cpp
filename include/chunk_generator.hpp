#ifndef CHUNK_GENERATOR_HPP
#define CHUNK_GENERATOR_HPP

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_set>
#include <unordered_map>

#include <condition_variable>
#include <shared_mutex>
#include <thread>

#include <stddef.h>

struct ChunkInfo;
struct ChunkGenerator
{
public:
  ChunkGenerator(std::size_t seed);
  ~ChunkGenerator();

public:
  const ChunkInfo *try_get_chunk_info(glm::ivec2 chunk_position) const;
  ChunkInfo *try_get_chunk_info(glm::ivec2 chunk_position);

private:
  void work(std::stop_token stoken);

private:
  std::size_t m_seed;

private:
  mutable std::shared_mutex           m_mutex;
  mutable std::condition_variable_any m_cv;

  mutable std::unordered_set<glm::ivec2>    m_pending_chunk_infos;
  mutable std::unordered_set<glm::ivec2>    m_loading_chunk_infos;
  std::unordered_map<glm::ivec2, ChunkInfo> m_chunk_infos;

private:
  std::vector<std::jthread> m_workers;
};

#endif // CHUNK_GENERATOR_HPP
