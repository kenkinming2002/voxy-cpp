#ifndef CHUNK_DEFS_HPP
#define CHUNK_DEFS_HPP

#include <glm/glm.hpp>

static constexpr int CHUNK_WIDTH  = 16;
static constexpr int CHUNK_HEIGHT = 256;

static constexpr int CHUNK_LOAD_RADIUS = 2;

static constexpr int CAVE_WORM_MAX = 2;
static constexpr int CAVE_WORM_SEGMENT_MAX = 10;

static constexpr float CAVE_WORM_MIN_HEIGHT = 10.0;
static constexpr float CAVE_WORM_MAX_HEIGHT = 30.0;

static constexpr float CAVE_WORM_MIN_RADIUS = 2.0;
static constexpr float CAVE_WORM_MAX_RADIUS = 5.0;

static constexpr float CAVE_WORM_STEP = 5.0f;

static constexpr glm::ivec3 DIRECTIONS[] = {
  {-1, 0, 0},
  { 1, 0, 0},
  {0, -1, 0},
  {0,  1, 0},
  {0, 0, -1},
  {0, 0,  1},
};

#endif // CHUNK_DEFS_HPP
