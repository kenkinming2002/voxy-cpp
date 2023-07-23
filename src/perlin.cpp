#include <perlin.hpp>

#include <glm/gtx/hash.hpp>

#include <random>

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

static float interpolate(float a0, float a1, float t)
{
  float factor = 6  * std::pow(t, 5)
               - 15 * std::pow(t, 4)
               + 10 * std::pow(t, 3);
  return a0 + (a1 - a0) * factor;
}

static glm::vec2 perlin_gradient(size_t seed, glm::ivec2 node)
{
  hash_combine(seed, node);

  std::mt19937                          prng(seed);
  std::uniform_real_distribution<float> dist(0.0f, 2.0f * M_PI);

  float a = dist(prng);
  return glm::vec2(std::cos(a), std::sin(a));
}

static glm::vec3 perlin_gradient(size_t seed, glm::ivec3 node)
{
  hash_combine(seed, node);

  std::mt19937                          prng(seed);
  std::uniform_real_distribution<float> dist_z(-1.0, 1.0);
  std::uniform_real_distribution<float> dist_a(0.0f, 2.0f * M_PI);

  float z = dist_z(prng);
  float a = dist_a(prng);
  float r = std::sqrt(1.0-z*z);
  return glm::vec3(r*std::cos(a), r*std::sin(a), z);
}

static float perlin(size_t seed, glm::vec2 pos)
{
  float influences[2][2];
  for(int cy=0; cy<2; ++cy)
      for(int cx=0; cx<2; ++cx)
      {
        glm::ivec2 anchor = glm::floor(pos);
        glm::ivec2 corner = anchor + glm::ivec2(cx, cy);

        glm::vec2 gradient = perlin_gradient(seed, corner);
        glm::vec2 offset   = pos - glm::vec2(corner);

        influences[cy][cx] = glm::dot(gradient, offset);
      }

  glm::vec2 factor = glm::fract(pos);
  float noise = interpolate(
    interpolate(influences[0][0], influences[0][1], factor.x),
    interpolate(influences[1][0], influences[1][1], factor.x),
    factor.y
  );
  return (noise + 1.0f) * 0.5f;
}

static float perlin(size_t seed, glm::vec3 pos)
{
  float influences[2][2][2];
  for(int cz=0; cz<2; ++cz)
    for(int cy=0; cy<2; ++cy)
      for(int cx=0; cx<2; ++cx)
      {
        glm::ivec3 anchor = glm::floor(pos);
        glm::ivec3 corner = anchor + glm::ivec3(cx, cy, cz);

        glm::vec3 gradient = perlin_gradient(seed, corner);
        glm::vec3 offset   = pos - glm::vec3(corner);

        influences[cz][cy][cx] = glm::dot(gradient, offset);
      }

  glm::vec3 factor = glm::fract(pos);
  float noise = interpolate(
    interpolate(
      interpolate(influences[0][0][0], influences[0][0][1], factor.x),
      interpolate(influences[0][1][0], influences[0][1][1], factor.x),
      factor.y
    ),
    interpolate(
      interpolate(influences[1][0][0], influences[1][0][1], factor.x),
      interpolate(influences[1][1][0], influences[1][1][1], factor.x),
      factor.y
    ),
    factor.z
  );
  return (noise + 1.0f) * 0.5f;
}

float perlin(size_t seed, glm::vec2 pos, float frequency, float amplitude)
{
  return perlin(seed, pos * frequency) * amplitude;
}

float perlin(size_t seed, glm::vec3 pos, float frequency, float amplitude)
{
  return perlin(seed, pos * frequency) * amplitude;
}

