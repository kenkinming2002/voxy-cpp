#include <chunk.hpp>
#include <chunk_coords.hpp>

void Chunk::update(const std::vector<BlockData>& block_datas)
{
  if(mesh_invalidated)
  {
    mesh_invalidated = false;
    remash(block_datas);
  }
}

std::optional<Block> Chunk::get_block(glm::ivec3 position) const
{
  assert(data);

  if(position.x < 0 || position.x >= CHUNK_WIDTH) return std::nullopt;
  if(position.y < 0 || position.y >= CHUNK_WIDTH) return std::nullopt;
  if(position.z < 0)                              return std::nullopt;

  if(position.z >= data->layers.size())
    return Block{ .presence = false };
  else
    return data->layers[position.z].blocks[position.y][position.x];
}

bool Chunk::set_block(glm::ivec3 position, Block block)
{
  assert(data);

  if(position.x < 0 || position.x >= CHUNK_WIDTH) return false;
  if(position.y < 0 || position.y >= CHUNK_WIDTH) return false;
  if(position.z < 0)                              return false;

  if(position.z >= data->layers.size())
    data->layers.resize(position.z+1); // NOTE: This works since value-initialization would set Block::presence to 0

  data->layers[position.z].blocks[position.y][position.x] = block;
  return true;
}

void Chunk::explode(glm::vec3 center, float radius)
{
  // TODO: Culling
  glm::ivec3 corner1 = glm::floor(center - glm::vec3(radius, radius, radius));
  glm::ivec3 corner2 = glm::ceil (center + glm::vec3(radius, radius, radius));
  for(int z = corner1.z; z<=corner2.z; ++z)
    for(int y = corner1.y; y<=corner2.y; ++y)
      for(int x = corner1.x; x<=corner2.x; ++x)
      {
        glm::ivec3 pos = { x, y, z };
        if(glm::length2(glm::vec3(pos) - center) < radius * radius)
          set_block(pos, Block{ .presence = false });
      }
}

void Chunk::generate(glm::ivec2 chunk_position, const ChunkGenerator& chunk_generator)
{
  assert(!data);
  data = std::make_unique<ChunkData>();

  const ChunkInfo* chunk_info = chunk_generator.try_get_chunk_info(chunk_position);
  assert(chunk_info);

  // 1: Create terrain based on height maps
  int max_height = 0;
  for(int ly=0; ly<CHUNK_WIDTH; ++ly)
    for(int lx=0; lx<CHUNK_WIDTH; ++lx)
    {
      int total_height = chunk_info->stone_height_map.heights[ly][lx]
                       + chunk_info->grass_height_map.heights[ly][lx];
      max_height = std::max(max_height, total_height);
    }

  data->layers.reserve(max_height);
  for(int lz=0; lz<max_height; ++lz)
  {
    ChunkData::Layer layer;
    for(int ly=0; ly<CHUNK_WIDTH; ++ly)
      for(int lx=0; lx<CHUNK_WIDTH; ++lx)
      {
        int height1 = chunk_info->stone_height_map.heights[ly][lx];
        int height2 = chunk_info->stone_height_map.heights[ly][lx] + chunk_info->grass_height_map.heights[ly][lx];
        layer.blocks[ly][lx] = lz < height1 ? Block::STONE :
                               lz < height2 ? Block::GRASS :
                                              Block::NONE;
      }
    data->layers.push_back(layer);
  }

  // 2: Carve out caves based off worms
  int        radius  = std::ceil(CAVE_WORM_SEGMENT_MAX * CAVE_WORM_STEP / CHUNK_WIDTH);
  glm::ivec2 corner1 = chunk_position - glm::ivec2(radius, radius);
  glm::ivec2 corner2 = chunk_position + glm::ivec2(radius, radius);
  for(int cy = corner1.y; cy <= corner2.y; ++cy)
    for(int cx = corner1.x; cx <= corner2.x; ++cx)
    {
      glm::ivec2       neighbour_chunk_position = glm::ivec2(cx, cy);
      const ChunkInfo *neighbour_chunk_info     = chunk_generator.try_get_chunk_info(neighbour_chunk_position);
      assert(neighbour_chunk_info);

      for(const Worm& worm : neighbour_chunk_info->worms)
        for(const Worm::Node& node : worm.nodes)
          explode(global_to_local(node.center, chunk_position), node.radius);
    }
}

void Chunk::remash(const std::vector<BlockData>& block_datas)
{
  if(mesh)
    mesh.reset();

  struct Vertex
  {
    glm::vec3 position;
    glm::vec2 texture_coords;
    uint32_t  texture_index;
    float     light_level;
  };

  std::vector<uint32_t> indices;
  std::vector<Vertex>   vertices;
  for(int lz=0; lz<height(); ++lz)
    for(int ly=0; ly<width(); ++ly)
      for(int lx=0; lx<width(); ++lx)
      {
        glm::ivec3 position  = { lx, ly, lz };
        Block      block     = get_block(position).value();
        if(!block.presence)
          continue;

        for(int i=0; i<std::size(DIRECTIONS); ++i)
        {
          glm::ivec3 direction          = DIRECTIONS[i];
          glm::ivec3 neighbour_position = position + direction;
          Block      neighbour_block    = get_block(neighbour_position).value_or(Block{.presence = 0}); // TODO: Actually lookup block in adjacent chunks
          if(neighbour_block.presence)
            continue;

          uint32_t index_base = vertices.size();
          indices.push_back(index_base + 0);
          indices.push_back(index_base + 1);
          indices.push_back(index_base + 2);
          indices.push_back(index_base + 2);
          indices.push_back(index_base + 1);
          indices.push_back(index_base + 3);

          glm::ivec3 out   = direction;
          glm::ivec3 up    = direction.z == 0.0 ? glm::ivec3(0, 0, 1) : glm::ivec3(1, 0, 0);
          glm::ivec3 right = glm::cross(glm::vec3(up), glm::vec3(out));
          glm::vec3 center = glm::vec3(position) + glm::vec3(0.5f, 0.5f, 0.5f) + 0.5f * glm::vec3(out);

          const BlockData& block_data = block_datas.at(block.id);
          vertices.push_back(Vertex{ .position = center + ( - 0.5f * glm::vec3(right) - 0.5f * glm::vec3(up)), .texture_coords = {0.0f, 0.0f}, .texture_index = block_data.texture_indices[i], .light_level = neighbour_block.light_level / 16.0f, });
          vertices.push_back(Vertex{ .position = center + ( + 0.5f * glm::vec3(right) - 0.5f * glm::vec3(up)), .texture_coords = {1.0f, 0.0f}, .texture_index = block_data.texture_indices[i], .light_level = neighbour_block.light_level / 16.0f, });
          vertices.push_back(Vertex{ .position = center + ( - 0.5f * glm::vec3(right) + 0.5f * glm::vec3(up)), .texture_coords = {0.0f, 1.0f}, .texture_index = block_data.texture_indices[i], .light_level = neighbour_block.light_level / 16.0f, });
          vertices.push_back(Vertex{ .position = center + ( + 0.5f * glm::vec3(right) + 0.5f * glm::vec3(up)), .texture_coords = {1.0f, 1.0f}, .texture_index = block_data.texture_indices[i], .light_level = neighbour_block.light_level / 16.0f, });
          // NOTE: Brackets added so that it is possible for the compiler to do constant folding if loop is unrolled, not that it would actually do it.
        }
      }

  MeshLayout layout{
    .index_type = IndexType::UNSIGNED_INT,
    .stride = sizeof(Vertex),
    .attributes = {
      { .type = AttributeType::FLOAT3,        .offset = offsetof(Vertex, position),       },
      { .type = AttributeType::FLOAT2,        .offset = offsetof(Vertex, texture_coords), },
      { .type = AttributeType::UNSIGNED_INT1, .offset = offsetof(Vertex, texture_index),  },
      { .type = AttributeType::FLOAT1,        .offset = offsetof(Vertex, light_level),    },
    },
  };

  this->mesh = std::make_unique<Mesh>(
      std::move(layout),
      as_bytes(indices),
      as_bytes(vertices)
  );
}

void Chunk::invalidate_mesh()
{
  mesh_invalidated = true;
}

