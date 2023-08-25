#pragma once

#include <perlin.hpp>

#include <glm/glm.hpp>

struct NoiseConfig
{
  float frequency;
  float amplitude;
  float lacunarity;
  float persistence;
  float octaves;
};

template<glm::length_t L>
float noise(size_t seed, glm::vec<L, float> position, NoiseConfig config)
{
  return perlin(seed, position, config.frequency, config.amplitude, config.lacunarity, config.persistence, config.octaves);
}
