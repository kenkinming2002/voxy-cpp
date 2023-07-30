#include <chunk_generator.hpp>
#include <chunk_info.hpp>

#include <spdlog/spdlog.h>

ChunkGenerator::ChunkGenerator(std::size_t seed)
  : m_seed(seed)
{
  unsigned count = std::thread::hardware_concurrency();
  for(unsigned i=0; i<count; ++i)
    m_workers.emplace_back(std::bind(&ChunkGenerator::work, this, std::placeholders::_1));
}
ChunkGenerator::~ChunkGenerator() {}

const ChunkInfo *ChunkGenerator::try_get_chunk_info(glm::ivec2 chunk_position) const
{
  {
    std::lock_guard guard(m_mutex);
    if(auto it = m_chunk_infos.find(chunk_position); it != m_chunk_infos.end())
      return &it->second;

    if(m_pending_chunk_infos.contains(chunk_position)) return nullptr;
    if(m_loading_chunk_infos.contains(chunk_position)) return nullptr;
    m_pending_chunk_infos.insert(chunk_position);
  }
  m_cv.notify_one();
  return nullptr;
}

ChunkInfo *ChunkGenerator::try_get_chunk_info(glm::ivec2 chunk_position)
{
  {
    std::lock_guard guard(m_mutex);
    if(auto it = m_chunk_infos.find(chunk_position); it != m_chunk_infos.end())
      return &it->second;

    if(m_pending_chunk_infos.contains(chunk_position)) return nullptr;
    if(m_loading_chunk_infos.contains(chunk_position)) return nullptr;
    m_pending_chunk_infos.insert(chunk_position);
  }
  m_cv.notify_one();
  return nullptr;
}

void ChunkGenerator::work(std::stop_token stoken)
{
  std::unique_lock lk(m_mutex);
  for(;;)
  {
    m_cv.wait(lk, stoken, [this]() { return !m_pending_chunk_infos.empty(); });
    if(stoken.stop_requested())
      return;

    while(!m_pending_chunk_infos.empty())
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
      if(stoken.stop_requested())
        return;
    }
  }
}

