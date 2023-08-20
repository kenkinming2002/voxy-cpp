#pragma once

#include <world.hpp>
#include <world_config.hpp>

#include <condition_variable>
#include <mutex>
#include <thread>
#include <unordered_map>

struct HeightMap
{
  float heights[CHUNK_WIDTH][CHUNK_WIDTH];
};

struct Worm
{
  struct Node
  {
    glm::vec3 center;
    float     radius;
  };
  std::vector<Node> nodes;
};

struct ChunkInfo
{
  std::vector<HeightMap> height_maps;
  std::vector<Worm>      worms;
};

class WorldGenerator
{
public:
  static constexpr size_t CHUNK_LOAD_RADIUS = 4;

public:
  WorldGenerator(WorldGenerationConfig config);

public:
  void update(World& world);

private:
  void try_load(World& world, glm::ivec2 chunk_index, int radius);
  void try_load(World& world, glm::ivec2 chunk_index);

private:
  bool ensure_chunk_info(glm::ivec2 chunk_index);
  const ChunkInfo& get_chunk_info(glm::ivec2 chunk_index) const;

private:
  WorldGenerationConfig m_config;

private:
  mutable std::mutex                  m_mutex;
  mutable std::condition_variable_any m_cv;

private:
  std::unordered_set<glm::ivec2>            m_pendings;
  std::unordered_set<glm::ivec2>            m_workings;
  std::unordered_map<glm::ivec2, ChunkInfo> m_chunk_infos;

private:
  std::vector<std::jthread> m_workers;
};
