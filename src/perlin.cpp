#include <perlin.hpp>

#include <glm/gtx/hash.hpp>

#include <random>

static float interpolate(float a0, float a1, float t)
{
  float factor = 6  * std::pow(t, 5)
               - 15 * std::pow(t, 4)
               + 10 * std::pow(t, 3);
  return a0 + (a1 - a0) * factor;
}

static glm::vec2 perlin_gradient(glm::ivec2 node)
{
  std::mt19937                          prng(std::hash<glm::ivec2>{}(node));
  std::uniform_real_distribution<float> dist(0.0f, 2.0f * M_PI);

  float a = dist(prng);
  return glm::vec2(std::cos(a), std::sin(a));
}

static float perlin(glm::vec2 pos)
{
  float influences[2][2];
  for(int cy=0; cy<2; ++cy)
      for(int cx=0; cx<2; ++cx)
      {
        glm::ivec2 anchor = glm::floor(pos);
        glm::ivec2 corner = anchor + glm::ivec2(cx, cy);

        glm::vec2 gradient = perlin_gradient(corner);
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

float perlin(glm::vec2 pos, float frequency, float amplitude)
{
  return perlin(pos * frequency) * amplitude;
}

