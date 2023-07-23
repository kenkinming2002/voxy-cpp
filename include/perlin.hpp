#ifndef PERLIN_HPP
#define PERLIN_HPP

#include <glm/glm.hpp>

float perlin(size_t seed, glm::vec2 pos, float frequency, float amplitude);
float perlin(size_t seed, glm::vec3 pos, float frequency, float amplitude);

#endif // PERLIN_HPP
