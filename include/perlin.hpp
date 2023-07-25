#ifndef PERLIN_HPP
#define PERLIN_HPP

#include <glm/glm.hpp>

float perlin(size_t seed, glm::vec2 pos, float frequency, float amplitude, float lacunarity, float presistence, unsigned count);
float perlin(size_t seed, glm::vec3 pos, float frequency, float amplitude, float lacunarity, float presistence, unsigned count);

#endif // PERLIN_HPP
