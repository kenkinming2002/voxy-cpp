#ifndef CHUNK_INFO_HPP
#define CHUNK_INFO_HPP

#include <chunk_defs.hpp>

#include <glm/glm.hpp>

#include <vector>

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
  HeightMap         stone_height_map;
  HeightMap         grass_height_map;
  std::vector<Worm> worms;
};

ChunkInfo generate_chunk_info(glm::ivec2 chunk_position, size_t seed);

#endif // CHUNK_INFO_HPP
