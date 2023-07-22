#include <world.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <random>

#include <math.h>

static glm::vec2 perlin_gradient(glm::ivec2 node)
{
  std::mt19937                          prng(std::hash<glm::ivec2>{}(node));
  std::uniform_real_distribution<float> dist(0.0f, 2.0f * M_PI);

  float a = dist(prng);
  return glm::vec2(std::cos(a), std::sin(a));
}

static float interpolate(float a0, float a1, float t)
{
  float factor = 6  * std::pow(t, 5)
               - 15 * std::pow(t, 4)
               + 10 * std::pow(t, 3);
  return a0 + (a1 - a0) * factor;
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

void World::generate_chunk(glm::ivec3 cpos)
{
  Chunk chunk;

  int heights[Chunk::WIDTH][Chunk::WIDTH];
  for(int cy=0; cy<Chunk::WIDTH; ++cy)
    for(int cx=0; cx<Chunk::WIDTH; ++cx)
      heights[cy][cx] = perlin(glm::vec2(cpos.x, cpos.y) + glm::vec2(
          (float)cx / (float)Chunk::WIDTH,
          (float)cy / (float)Chunk::WIDTH
      )) * Chunk::WIDTH;

  for(int cz=0; cz<Chunk::WIDTH; ++cz)
    for(int cy=0; cy<Chunk::WIDTH; ++cy)
      for(int cx=0; cx<Chunk::WIDTH; ++cx)
      {
        if(cz <= heights[cx][cy])
          chunk.blocks[cz][cy][cx] = 1;
        else
          chunk.blocks[cz][cy][cx] = 0;
      }

  chunks.insert_or_assign(cpos, chunk);
}

void World::generate_chunk_mesh(glm::ivec3 cpos)
{
  Chunk& chunk = chunks.at(cpos);

  std::vector<uint32_t> indices;
  std::vector<Vertex>   vertices;
  for(int z=0; z<Chunk::WIDTH; ++z)
    for(int y=0; y<Chunk::WIDTH; ++y)
      for(int x=0; x<Chunk::WIDTH; ++x)
        if(chunk.blocks[z][y][x])
        {
          static constexpr uint32_t cube_indices[] = {
            0,  1,  2,  2,  1,  3,
            6,  5,  4,  7,  5,  6,
            10,  9,  8, 11, 9,  10,
            12, 13, 14, 14, 13, 15,
            16, 17, 18, 18, 17, 19,
            22, 21, 20, 23, 21, 22,
          };

          static constexpr Vertex cube_vertices[] = {
            {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 1.0f}},
            {{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}},
            {{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 1.0f}},
            {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}},

            {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
            {{0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
            {{1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
            {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},

            {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
            {{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},

            {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
            {{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},

            {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
            {{0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
            {{0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
            {{0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},

            {{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
            {{1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
            {{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
            {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
          };

          uint32_t index_base = vertices.size();
          for(uint32_t cube_index : cube_indices)
          {
            cube_index += index_base;
            indices.push_back(cube_index);
          }

          glm::vec3 pos_base = glm::vec3(x, y, z);
          for(Vertex cube_vertex : cube_vertices)
          {
            cube_vertex.pos += pos_base;
            vertices.push_back(cube_vertex);
          }
        }

  Mesh mesh(indices, vertices);
  chunk_meshes.insert_or_assign(cpos, std::move(mesh));
}

void World::draw(const Camera& camera)
{
  glm::mat4 view       = camera.view();
  glm::mat4 projection = camera.projection();
  for(const auto& [pos, chunk_mesh] : chunk_meshes)
  {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), (float)Chunk::WIDTH * glm::vec3(pos));
    glm::mat4 transform = projection * view * model;

    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(transform));
    chunk_mesh.draw();
  }
}
