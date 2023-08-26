#pragma once

#include <world.hpp>
#include <world_config.hpp>

#include <light_manager.hpp>

#include <lazy.hpp>

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
  WorldGenerator(GenerationConfig config);

public:
  void update(World& world, LightManager& light_manager);

private:
  void try_load(World& world, LightManager& light_manager, glm::ivec2 chunk_index, int radius);
  void try_load(World& world, LightManager& light_manager, glm::ivec2 chunk_index);

private:
  GenerationConfig                                m_config;
  std::unordered_map<glm::ivec2, Lazy<ChunkInfo>> m_chunk_infos;
};
