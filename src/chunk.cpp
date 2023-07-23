#include <chunk.hpp>

#include <perlin.hpp>

#include <optional>
#include <iostream>

static std::vector<Layer> generate_layers(glm::ivec2 cpos)
{
  int stone_heights[Layer::WIDTH][Layer::WIDTH];
  for(int cy=0; cy<Layer::WIDTH; ++cy)
    for(int cx=0; cx<Layer::WIDTH; ++cx)
    {
      glm::vec2 pos = glm::vec2(Layer::WIDTH * cpos) + glm::vec2(cx, cy);
      stone_heights[cy][cx] = 0.0f;
      stone_heights[cy][cx] += perlin(pos, 1.0f   / Layer::WIDTH, 5.0f);
      stone_heights[cy][cx] += perlin(pos, 0.5f   / Layer::WIDTH, 10.0f);
      stone_heights[cy][cx] += perlin(pos, 0.25f  / Layer::WIDTH, 20.0f);
      stone_heights[cy][cx] += perlin(pos, 0.125f / Layer::WIDTH, 40.0f);
    }

  int grass_heights[Layer::WIDTH][Layer::WIDTH];
  for(int cy=0; cy<Layer::WIDTH; ++cy)
    for(int cx=0; cx<Layer::WIDTH; ++cx)
    {
      glm::vec2 pos = glm::vec2(Layer::WIDTH * cpos) + glm::vec2(cx, cy);
      grass_heights[cy][cx] = 0.0f;
      grass_heights[cy][cx] += perlin(pos, 0.25f / Layer::WIDTH, 1.25f);
      grass_heights[cy][cx] += perlin(pos, 0.5f  / Layer::WIDTH, 2.5f);
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
            .color    = glm::vec3(0.7, 0.7, 0.7),
          };
        else if(cz <= stone_heights[cy][cx] + grass_heights[cy][cx])
          layer.blocks[cy][cx] = Block {
            .presence = true,
            .color    = glm::vec3(0.2, 1.0, 0.2),
          };
        else
          layer.blocks[cy][cx] = Block {
            .presence = false,
          };

    layers.push_back(layer);
  }


  // Hopefully carve some caves
  for(int cz=0; cz<max_height; ++cz)
    for(int cy=0; cy<Layer::WIDTH; ++cy)
      for(int cx=0; cx<Layer::WIDTH; ++cx)
      {
        Block& block = layers[cz].blocks[cy][cx];
        glm::vec3 pos = glm::vec3(
            cpos.x * Layer::WIDTH + cx,
            cpos.y * Layer::WIDTH + cy,
            cz
        );
        float noise = 0.0f;
        noise += perlin(pos, 16.0 / Layer::WIDTH, 0.03125f);
        noise += perlin(pos, 8.0  / Layer::WIDTH, 0.0625f);
        noise += perlin(pos, 4.0  / Layer::WIDTH, 0.125f);
        noise += perlin(pos, 2.0  / Layer::WIDTH, 0.25f);
        noise += perlin(pos, 1.0  / Layer::WIDTH, 0.5f);
        if(noise<=0.4)
          block.presence = false;
      }

  return layers;
}

struct Face
{
  static constexpr size_t INDEX_COUNT  = 6;
  static constexpr size_t VERTEX_COUNT = 4;

  static constexpr uint32_t INDICES[INDEX_COUNT] = {0, 1, 2, 2, 1, 3};
  static constexpr glm::vec2 UVS[VERTEX_COUNT] = {
    {0.0, 0.0},
    {1.0, 0.0},
    {0.0, 1.0},
    {1.0, 1.0},
  };
  glm::vec3 positions[VERTEX_COUNT];
  glm::vec3 normal;

  glm::ivec3 dir;
};

static constexpr Face FACE_NEGATIVE_X = {
  .positions = {
    {-0.5f,  0.5f, -0.5f},
    {-0.5f, -0.5f, -0.5f},
    {-0.5f,  0.5f,  0.5f},
    {-0.5f, -0.5f,  0.5f},
  },
  .normal = {-1.0f, 0.0f, 0.0f},
  .dir = {-1, 0, 0},
};

static constexpr Face FACE_POSITIVE_X = {
  .positions = {
    {0.5f, -0.5f, -0.5f},
    {0.5f,  0.5f, -0.5f},
    {0.5f, -0.5f,  0.5f},
    {0.5f,  0.5f,  0.5f},
  },
  .normal = {1.0f, 0.0f, 0.0f},
  .dir = {1, 0, 0},
};

static constexpr Face FACE_NEGATIVE_Y = {
  .positions = {
    {-0.5f, -0.5f, -0.5f},
    { 0.5f, -0.5f, -0.5f},
    {-0.5f, -0.5f,  0.5f},
    { 0.5f, -0.5f,  0.5f},
  },
  .normal = {0.0f, -1.0f, 0.0f},
  .dir = {0, -1, 0},
};

static constexpr Face FACE_POSITIVE_Y = {
  .positions = {
    { 0.5f, 0.5f, -0.5f},
    {-0.5f, 0.5f, -0.5f},
    { 0.5f, 0.5f,  0.5f},
    {-0.5f, 0.5f,  0.5f},
  },
  .normal = {0.0f, 1.0f, 0.0f},
  .dir = {0, 1, 0},
};

static constexpr Face FACE_NEGATIVE_Z = {
  .positions = {
    {-0.5f,  0.5f, -0.5f},
    { 0.5f,  0.5f, -0.5f},
    {-0.5f, -0.5f, -0.5f},
    { 0.5f, -0.5f, -0.5f},
  },
  .normal = {0.0f, 0.0f, -1.0f},
  .dir = {0, 0, -1},
};

static constexpr Face FACE_POSITIVE_Z = {
  .positions = {
    {-0.5f, -0.5f, 0.5f},
    { 0.5f, -0.5f, 0.5f},
    {-0.5f,  0.5f, 0.5f},
    { 0.5f,  0.5f, 0.5f},
  },
  .normal = {0.0f, 0.0f, 1.0f},
  .dir = {0, 0, 1},
};

static constexpr Face FACES[] = {
  FACE_NEGATIVE_X,
  FACE_POSITIVE_X,
  FACE_NEGATIVE_Y,
  FACE_POSITIVE_Y,
  FACE_NEGATIVE_Z,
  FACE_POSITIVE_Z,
};

struct Vertex
{
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 color;
};

static void add_face(std::vector<uint32_t>& indices, std::vector<Vertex>& vertices, const Face& face, glm::vec3 position, glm::vec3 color)
{
  uint32_t index_base = vertices.size();
  for(uint32_t index : Face::INDICES)
    indices.push_back(index_base + index);

  for(size_t i=0; i<Face::VERTEX_COUNT; ++i)
    vertices.push_back(Vertex{
      .position = position + face.positions[i],
      .normal   = face.normal,
      .color    = color,
    });
}

static std::optional<Block> layers_get_block(const std::vector<Layer>& layers, glm::ivec3 position)
{
  if(position.x < 0 || position.x >= Layer::WIDTH)  return std::nullopt;
  if(position.y < 0 || position.y >= Layer::WIDTH)  return std::nullopt;
  if(position.y < 0 || position.z >= layers.size()) return Block{ .presence = false };
  return layers[position.z].blocks[position.y][position.x];
}

static Mesh generate_layers_mesh(glm::ivec2 cpos, const std::vector<Layer>& layers)
{
  std::vector<uint32_t> indices;
  std::vector<Vertex>   vertices;
  for(int z=0; z<layers.size(); ++z)
    for(int y=0; y<Layer::WIDTH; ++y)
      for(int x=0; x<Layer::WIDTH; ++x)
      {
        const Block& block = layers[z].blocks[y][x];
        glm::ivec3 position = {x, y, z};
        if(block.presence)
          for(Face face : FACES)
          {
            auto adjacent_block = layers_get_block(layers, position + face.dir);
            if(adjacent_block && adjacent_block->presence)
              continue; // Culling

            add_face(indices, vertices, face, position, block.color);
          }
      }

  return Mesh(indices, VertexLayout{
    .stride = sizeof(Vertex),
    .attributes = {
      { .offset = offsetof(Vertex, position), .count = 3, },
      { .offset = offsetof(Vertex, normal),   .count = 3, },
      { .offset = offsetof(Vertex, color),    .count = 3, },
    },
  }, std::as_bytes(std::span(vertices)));
}

Chunk::Chunk(glm::ivec2 cpos) :
  layers(generate_layers(cpos)),
  layers_mesh(generate_layers_mesh(cpos, layers)),
  layers_need_remash(false)
{}
