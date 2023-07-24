#include <light.hpp>

static constexpr uint32_t CUBE_INDICES[] = {
  0,  1,  2,  2,  1,  3,
  6,  5,  4,  7,  5,  6,
  10,  9,  8, 11, 9,  10,
  12, 13, 14, 14, 13, 15,
  16, 17, 18, 18, 17, 19,
  22, 21, 20, 23, 21, 22,
};

static constexpr std::pair<glm::vec3, glm::vec3> CUBE_VERTICES[] = {
  {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
  {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
  {{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
  {{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},

  {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
  {{0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
  {{1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
  {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},

  {{0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
  {{0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
  {{1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
  {{1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},

  {{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
  {{0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
  {{1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
  {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},

  {{0.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
  {{0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},
  {{0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
  {{0.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},

  {{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
  {{1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
  {{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
  {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
};

static Mesh build_light_mesh()
{
  struct Vertex
  {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec3 color;
  };

  std::vector<uint32_t> indices;
  std::vector<Vertex>   vertices;

  for(uint32_t cube_index : CUBE_INDICES)
    indices.push_back(cube_index);

  for(auto [cube_position, cube_normal] : CUBE_VERTICES)
    vertices.push_back(Vertex{
        .pos    = cube_position,
        .normal = cube_normal,
        .color  = glm::vec3(1.0, 1.0, 1.0),
    });

  return Mesh(indices, VertexLayout{
    .stride = sizeof(Vertex),
    .attributes = {
      { .offset = offsetof(Vertex, pos),    .count = 3, },
      { .offset = offsetof(Vertex, normal), .count = 3, },
      { .offset = offsetof(Vertex, color),  .count = 3, },
    },
  }, std::as_bytes(std::span(vertices)));
}


Light::Light(glm::vec3 pos, glm::vec3 ambient, glm::vec3 diffuse) :
  pos(pos),
  ambient(ambient),
  diffuse(diffuse),
  mesh(build_light_mesh())
{}
