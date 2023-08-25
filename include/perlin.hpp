#ifndef PERLIN_HPP
#define PERLIN_HPP

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/norm.hpp>

#include <random>

namespace details
{
  template <class T>
  static inline void hash_combine(std::size_t& seed, const T& v)
  {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
  }

  template<glm::length_t L, glm::length_t D>
  static inline glm::vec<L, int> unit_vector()
  {
    glm::vec<L, int> vector(0);
    vector[D] = 1;
    return vector;
  }

  template<glm::length_t L>
  static inline glm::vec<L, float> perlin_gradient(size_t seed, glm::vec<L, int> node)
  {
    hash_combine(seed, node);

    std::mt19937                    prng(seed);
    std::normal_distribution<float> dist(0.0f, 1.0f);

    glm::vec<L, float> result;
    for(glm::length_t i=0; i<L; ++i)
      result[i] = dist(prng);

    if(glm::length2(result) != 0.0f)
      result = glm::normalize(result);

    return result;
  }

  template<glm::length_t L, glm::length_t D>
  static inline float perlin_impl(size_t seed, glm::vec<L, float> position, glm::vec<L, int> node) requires(D == L)
  {
    glm::vec<L, float> gradient = perlin_gradient(seed, node);
    glm::vec<L, float> offset   = position - glm::vec<L, float>(node);
    return glm::dot(gradient, offset);
  }

  template<glm::length_t L, glm::length_t D>
  static inline float perlin_impl(size_t seed, glm::vec<L, float> position, glm::vec<L, int> node) requires(0 <= D && D < L)
  {
    float a = perlin_impl<L, D+1>(seed, position, node);
    float b = perlin_impl<L, D+1>(seed, position, node + unit_vector<L, D>());
    float t = position[D] - node[D];
    float factor = 6  * std::pow(t, 5)
      - 15 * std::pow(t, 4)
      + 10 * std::pow(t, 3);
    return a + (b - a) * factor;
  }
}

template<glm::length_t L>
float perlin(size_t seed, glm::vec<L, float> position)
{
  glm::vec<L, int> node = glm::floor(position);
  return details::perlin_impl<L, 0>(seed, position, node);
}

template<glm::length_t L>
float perlin(size_t seed, glm::vec<L, float> position, float frequency, float amplitude, float lacunarity, float persistence, unsigned octaves)
{
  float value = 0.0f;
  for(unsigned i=0; i<octaves; ++i)
  {
    value += perlin(seed, position * frequency) * amplitude;
    frequency *= lacunarity;
    amplitude *= persistence;
  }
  return value;
}

#endif // PERLIN_HPP
