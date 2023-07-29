#include <chunk_info.hpp>
#include <chunk_coords.hpp>

#include <perlin.hpp>

#include <glm/gtx/hash.hpp>
#include <glm/gtx/norm.hpp>

#include <random>

template <class T>
static size_t hash_combine(std::size_t seed, const T& v)
{
    std::hash<T> hasher;
    return seed ^ (hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2));
}

static HeightMap generate_height_map(glm::ivec2 chunk_position, std::mt19937& prng, float frequency, float amplitude, float lacunarity, float presistence, unsigned count)
{
  size_t seed = prng();

  HeightMap height_map;
  for(int ly=0; ly<CHUNK_WIDTH; ++ly)
    for(int lx=0; lx<CHUNK_WIDTH; ++lx)
    {
      glm::ivec2 position = local_to_global(glm::ivec2(lx, ly), chunk_position);
      height_map.heights[ly][lx] = perlin(seed, position, frequency, amplitude, lacunarity, presistence, count);
    }

  return height_map;
}

static std::vector<Worm> generate_worms(glm::ivec2 chunk_position, std::mt19937& prng)
{
  size_t seed_x      = prng();
  size_t seed_y      = prng();
  size_t seed_z      = prng();
  size_t seed_radius = prng();

  std::vector<Worm> worms;

  int worm_count = std::uniform_int_distribution<int>(0, CAVE_WORM_MAX)(prng);
  for(int i=0; i<worm_count; ++i)
  {
    Worm worm;

    glm::vec3 local_origin;
    local_origin.x = std::uniform_real_distribution<float>(0, CHUNK_WIDTH-1)(prng);
    local_origin.y = std::uniform_real_distribution<float>(0, CHUNK_WIDTH-1)(prng);
    local_origin.z = std::uniform_real_distribution<float>(CAVE_WORM_MIN_HEIGHT, CAVE_WORM_MAX_HEIGHT)(prng);
    glm::vec3 origin = local_to_global(local_origin, chunk_position);

    glm::vec3 position = origin;
    for(unsigned i=0; i<CAVE_WORM_SEGMENT_MAX; ++i)
    {
      // TODO: Consider implementing overloads of perlin noise that allow specifying min/max

      // 1: Record the node
      Worm::Node node;
      node.center = position;
      node.radius = CAVE_WORM_MIN_RADIUS + perlin(seed_radius, position, 0.1f, CAVE_WORM_MAX_RADIUS-CAVE_WORM_MIN_RADIUS, 2.0f, 0.5f, 1);
      worm.nodes.push_back(node);

      // 2: Advance the worm
      glm::vec3 direction;
      direction.x = perlin(seed_x, position, 0.1f, 1.0f, 2.0f, 0.5f, 4) - 1.0f;
      direction.y = perlin(seed_y, position, 0.1f, 1.0f, 2.0f, 0.5f, 4) - 1.0f;
      direction.z = perlin(seed_z, position, 0.1f, 1.0f, 2.0f, 0.5f, 4) - 1.0f;
      if(glm::length2(direction) < 1e-4)
        direction = glm::vec3(0.0f, 0.0f, 1.0f);

      position += CAVE_WORM_STEP * glm::normalize(direction);
    }

    worms.push_back(std::move(worm));
  }

  return worms;
}

ChunkInfo ChunkInfo::generate(glm::ivec2 chunk_position, size_t seed)
{
  std::mt19937 prng_global(seed);
  std::mt19937 prng_local(hash_combine(seed, chunk_position));

  HeightMap         stone_height_map = generate_height_map(chunk_position, prng_global, 0.03f, 40.0f, 2.0f, 0.5f, 4);
  HeightMap         grass_height_map = generate_height_map(chunk_position, prng_global, 0.01f, 5.0f,  2.0f, 0.5f, 2);
  std::vector<Worm> worms            = generate_worms(chunk_position, prng_local);

  return ChunkInfo {
    .stone_height_map = std::move(stone_height_map),
    .grass_height_map = std::move(grass_height_map),
    .worms            = std::move(worms),
  };
}

