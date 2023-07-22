#include <world.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

void World::generate_chunk(glm::ivec3 pos)
{
  Chunk chunk;
  for(size_t z=0; z<Chunk::WIDTH; ++z)
    for(size_t y=0; y<Chunk::WIDTH; ++y)
      for(size_t x=0; x<Chunk::WIDTH; ++x)
        chunk.blocks[z][y][x] = (x+y+z+1) % 2;
  chunks.insert_or_assign(pos, chunk);
}

void World::generate_chunk_mesh(glm::ivec3 pos)
{
  Chunk& chunk = chunks.at(pos);

  std::vector<uint32_t> indices;
  std::vector<Vertex>   vertices;
  for(size_t z=0; z<Chunk::WIDTH; ++z)
    for(size_t y=0; y<Chunk::WIDTH; ++y)
      for(size_t x=0; x<Chunk::WIDTH; ++x)
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
  chunk_meshes.insert_or_assign(pos, std::move(mesh));
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
