#include <chunk.hpp>

#include <perlin.hpp>

static std::vector<Layer> generate_layers(glm::ivec2 cpos)
{
  int stone_heights[Layer::WIDTH][Layer::WIDTH];
  for(int cy=0; cy<Layer::WIDTH; ++cy)
    for(int cx=0; cx<Layer::WIDTH; ++cx)
    {
      glm::vec2 pos = glm::vec2(Layer::WIDTH * cpos) + glm::vec2(cx, cy);
      stone_heights[cy][cx] = 0.0f;
      stone_heights[cy][cx] += perlin(pos, 0.1f  / Layer::WIDTH, 1.0f);
      stone_heights[cy][cx] += perlin(pos, 0.25f / Layer::WIDTH, 5.0f);
      stone_heights[cy][cx] += perlin(pos, 0.5f  / Layer::WIDTH, 10.0f);
      stone_heights[cy][cx] += perlin(pos, 1.0f  / Layer::WIDTH, 20.0f);
    }

  int grass_heights[Layer::WIDTH][Layer::WIDTH];
  for(int cy=0; cy<Layer::WIDTH; ++cy)
    for(int cx=0; cx<Layer::WIDTH; ++cx)
    {
      glm::vec2 pos = glm::vec2(Layer::WIDTH * cpos) + glm::vec2(cx, cy);
      grass_heights[cy][cx] = 0.0f;
      grass_heights[cy][cx] += perlin(pos, 0.5f  / Layer::WIDTH, 2.0f);
      grass_heights[cy][cx] += perlin(pos, 1.0f  / Layer::WIDTH, 5.0f);
    }

  int max_height = 0;
  for(int cy=0; cy<Layer::WIDTH; ++cy)
    for(int cx=0; cx<Layer::WIDTH; ++cx)
      if(max_height < stone_heights[cy][cx]+grass_heights[cy][cx])
        max_height = stone_heights[cy][cx]+grass_heights[cy][cx];

  std::vector<Layer> layers;
  for(int cz=0; cz<max_height; ++cz)
  {
    Layer layer;
    for(int cy=0; cy<Layer::WIDTH; ++cy)
      for(int cx=0; cx<Layer::WIDTH; ++cx)
        if(cz <= stone_heights[cy][cx])
          layer.blocks[cy][cx] = Block {
            .presence = true,
            .color    = glm::vec3(0.2, 1.0, 0.2),
          };
        else if(cz <= stone_heights[cy][cx] + grass_heights[cy][cx])
          layer.blocks[cy][cx] = Block {
            .presence = true,
            .color    = glm::vec3(0.7, 0.7, 0.7),
          };
        else
          layer.blocks[cy][cx] = Block {
            .presence = false,
          };

    layers.push_back(layer);
  }
  return layers;
}

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

static Mesh generate_layers_mesh(glm::ivec2 cpos, const std::vector<Layer>& layers)
{
  struct Vertex
  {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec3 color;
  };

  std::vector<uint32_t> indices;
  std::vector<Vertex>   vertices;
  for(int z=0; z<layers.size(); ++z)
    for(int y=0; y<Layer::WIDTH; ++y)
      for(int x=0; x<Layer::WIDTH; ++x)
      {
        const Block& block = layers[z].blocks[y][x];
        if(block.presence)
        {
          uint32_t index_base = vertices.size();
          for(uint32_t cube_index : CUBE_INDICES)
            indices.push_back(index_base + cube_index);

          glm::vec3 position_base = glm::vec3(x, y, z);
          for(auto [cube_position, cube_normal] : CUBE_VERTICES)
            vertices.push_back(Vertex{
                .pos    = position_base + cube_position,
                .normal = cube_normal,
                .color  = block.color,
            });
        }
      }

  return Mesh(indices, VertexLayout{
    .stride = sizeof(Vertex),
    .attributes = {
      { .offset = offsetof(Vertex, pos),    .count = 3, },
      { .offset = offsetof(Vertex, normal), .count = 3, },
      { .offset = offsetof(Vertex, color),  .count = 3, },
    },
  }, std::as_bytes(std::span(vertices)));
}

Chunk::Chunk(glm::ivec2 cpos) :
  layers(generate_layers(cpos)),
  layers_mesh(generate_layers_mesh(cpos, layers)),
  layers_need_remash(false)
{}
