#include <world.hpp>

#include <perlin.hpp>

#include <glm/gtx/norm.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <spdlog/spdlog.h>

#include <random>

/*************
 * Constants *
 *************/
static constexpr int CAVE_WORM_MAX = 2;
static constexpr int CAVE_WORM_SEGMENT_MAX = 10;

static constexpr float CAVE_WORM_MIN_HEIGHT = 10.0;
static constexpr float CAVE_WORM_MAX_HEIGHT = 30.0;

static constexpr float CAVE_WORM_MIN_RADIUS = 2.0;
static constexpr float CAVE_WORM_MAX_RADIUS = 5.0;

static constexpr float CAVE_WORM_STEP = 5.0f;

static constexpr int CHUNK_LOAD_RADIUS = 2;

static constexpr float     FRICTION = 0.5f;
static constexpr glm::vec3 GRAVITY  = glm::vec3(0.0f, 0.0f, -5.0f);

/*************
 * Utilities *
 *************/
template <class T>
inline size_t hash_combine(std::size_t seed, const T& v)
{
    std::hash<T> hasher;
    return seed ^ (hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2));
}

static glm::ivec3 global_to_local(glm::ivec3 position, glm::ivec2 chunk_position)
{
  return glm::ivec3(
      position.x - chunk_position.x * CHUNK_WIDTH,
      position.y - chunk_position.y * CHUNK_WIDTH,
      position.z
  );
}

static glm::vec3 global_to_local(glm::vec3 position, glm::ivec2 chunk_position)
{
  return glm::vec3(
      position.x - chunk_position.x * CHUNK_WIDTH,
      position.y - chunk_position.y * CHUNK_WIDTH,
      position.z
  );
}

static glm::ivec3 local_to_global(glm::ivec3 position, glm::ivec2 chunk_position)
{
  return glm::ivec3(
      position.x + chunk_position.x * CHUNK_WIDTH,
      position.y + chunk_position.y * CHUNK_WIDTH,
      position.z
  );
}

static glm::vec3 local_to_global(glm::vec3 position, glm::ivec2 chunk_position)
{
  return glm::vec3(
      position.x + chunk_position.x * CHUNK_WIDTH,
      position.y + chunk_position.y * CHUNK_WIDTH,
      position.z
  );
}

static glm::ivec2 global_to_local(glm::ivec2 position, glm::ivec2 chunk_position)
{
  return glm::ivec2(
      position.x - chunk_position.x * CHUNK_WIDTH,
      position.y - chunk_position.y * CHUNK_WIDTH
  );
}

static glm::vec2 global_to_local(glm::vec2 position, glm::ivec2 chunk_position)
{
  return glm::vec2(
      position.x - chunk_position.x * CHUNK_WIDTH,
      position.y - chunk_position.y * CHUNK_WIDTH
  );
}

static glm::ivec2 local_to_global(glm::ivec2 position, glm::ivec2 chunk_position)
{
  return glm::ivec2(
      position.x + chunk_position.x * CHUNK_WIDTH,
      position.y + chunk_position.y * CHUNK_WIDTH
  );
}

static glm::vec2 local_to_global(glm::vec2 position, glm::ivec2 chunk_position)
{
  return glm::vec2(
      position.x + chunk_position.x * CHUNK_WIDTH,
      position.y + chunk_position.y * CHUNK_WIDTH
  );
}

static glm::vec3 min_axis(glm::vec3 vec)
{
  float min = std::abs(vec.x);
  min = std::min(min, std::abs(vec.y));
  min = std::min(min, std::abs(vec.z));
  if(min != std::abs(vec.x)) vec.x = 0.0f;
  if(min != std::abs(vec.y)) vec.y = 0.0f;
  if(min != std::abs(vec.z)) vec.z = 0.0f;
  return vec;
}

static int modulo(int a, int b)
{
  int value = a % b;
  if(value < 0)
    value += b;

  assert(0 <= value);
  assert(value < b);
  return value;
}

/*************
 * ChunkInfo *
 *************/
static HeightMap generate_height_map(glm::ivec2 chunk_position, std::mt19937& prng, float frequency, float amplitude, float lacunarity, float presistence, unsigned count)
{
  size_t seed = prng();

  HeightMap height_map;
  for(int ly=0; ly<CHUNK_WIDTH; ++ly)
    for(int lx=0; lx<CHUNK_WIDTH; ++lx)
    {
      glm::ivec2 position = local_to_global(glm::ivec2(lx, ly), chunk_position);
      height_map.heights[ly][lx] = perlin(seed, position, frequency, amplitude, lacunarity, presistence, count);
    }

  return height_map;
}

static std::vector<Worm> generate_worms(glm::ivec2 chunk_position, std::mt19937& prng)
{
  size_t seed_x      = prng();
  size_t seed_y      = prng();
  size_t seed_z      = prng();
  size_t seed_radius = prng();

  std::vector<Worm> worms;

  int worm_count = std::uniform_int_distribution<int>(0, CAVE_WORM_MAX)(prng);
  for(int i=0; i<worm_count; ++i)
  {
    Worm worm;

    glm::vec3 local_origin;
    local_origin.x = std::uniform_real_distribution<float>(0, CHUNK_WIDTH-1)(prng);
    local_origin.y = std::uniform_real_distribution<float>(0, CHUNK_WIDTH-1)(prng);
    local_origin.z = std::uniform_real_distribution<float>(CAVE_WORM_MIN_HEIGHT, CAVE_WORM_MAX_HEIGHT)(prng);
    glm::vec3 origin = local_to_global(local_origin, chunk_position);

    glm::vec3 position = origin;
    for(unsigned i=0; i<CAVE_WORM_SEGMENT_MAX; ++i)
    {
      // TODO: Consider implementing overloads of perlin noise that allow specifying min/max

      // 1: Record the node
      Worm::Node node;
      node.center = position;
      node.radius = CAVE_WORM_MIN_RADIUS + perlin(seed_radius, position, 0.1f, CAVE_WORM_MAX_RADIUS-CAVE_WORM_MIN_RADIUS, 2.0f, 0.5f, 1);
      worm.nodes.push_back(node);

      // 2: Advance the worm
      glm::vec3 direction;
      direction.x = perlin(seed_x, position, 0.1f, 1.0f, 2.0f, 0.5f, 4) - 1.0f;
      direction.y = perlin(seed_y, position, 0.1f, 1.0f, 2.0f, 0.5f, 4) - 1.0f;
      direction.z = perlin(seed_z, position, 0.1f, 1.0f, 2.0f, 0.5f, 4) - 1.0f;
      if(glm::length2(direction) < 1e-4)
        direction = glm::vec3(0.0f, 0.0f, 1.0f);

      position += CAVE_WORM_STEP * glm::normalize(direction);
    }

    worms.push_back(std::move(worm));
  }

  return worms;
}

static ChunkInfo generate_chunk_info(glm::ivec2 chunk_position, size_t seed)
{
  std::mt19937 prng_global(seed);
  std::mt19937 prng_local(hash_combine(seed, chunk_position));

  HeightMap         stone_height_map = generate_height_map(chunk_position, prng_global, 0.03f, 40.0f, 2.0f, 0.5f, 4);
  HeightMap         grass_height_map = generate_height_map(chunk_position, prng_global, 0.01f, 5.0f,  2.0f, 0.5f, 2);
  std::vector<Worm> worms            = generate_worms(chunk_position, prng_local);

  return ChunkInfo {
    .stone_height_map = std::move(stone_height_map),
    .grass_height_map = std::move(grass_height_map),
    .worms            = std::move(worms),
  };
}

/**********
 * Blocks *
 **********/
static Block get_block(const ChunkData& chunk_data, glm::ivec3 position)
{
  if(position.x < 0 || position.x >= CHUNK_WIDTH)              return Block{ .presence = false };
  if(position.y < 0 || position.y >= CHUNK_WIDTH)              return Block{ .presence = false };
  if(position.z < 0 || position.z >= chunk_data.slices.size()) return Block{ .presence = false };
  return chunk_data.slices[position.z].blocks[position.y][position.x];
}

static void set_block(ChunkData& chunk_data, glm::ivec3 position, Block block)
{
  if(position.x < 0 || position.x >= CHUNK_WIDTH)              return;
  if(position.y < 0 || position.y >= CHUNK_WIDTH)              return;
  if(position.z < 0 || position.z >= chunk_data.slices.size()) return;
  chunk_data.slices[position.z].blocks[position.y][position.x] = block;
}

static void explode(ChunkData& chunk_data, glm::vec3 center, float radius)
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
          set_block(chunk_data, pos, Block{ .presence = false });
      }
}

static ChunkData generate_chunk_data(glm::ivec2 chunk_position, const std::unordered_map<glm::ivec2, ChunkInfo>& chunk_infos)
{
  const ChunkInfo& chunk_info = chunk_infos.at(chunk_position);

  ChunkData chunk_data;

  // 1: Create terrain based on height maps
  int max_height = 0;
  for(int ly=0; ly<CHUNK_WIDTH; ++ly)
    for(int lx=0; lx<CHUNK_WIDTH; ++lx)
    {
      int total_height = chunk_info.stone_height_map.heights[ly][lx]
                       + chunk_info.grass_height_map.heights[ly][lx];
      max_height = std::max(max_height, total_height);
    }

  chunk_data.slices.reserve(max_height);
  for(int lz=0; lz<max_height; ++lz)
  {
    ChunkData::Slice slice;
    for(int ly=0; ly<CHUNK_WIDTH; ++ly)
      for(int lx=0; lx<CHUNK_WIDTH; ++lx)
      {
        int height1 = chunk_info.stone_height_map.heights[ly][lx];
        int height2 = chunk_info.stone_height_map.heights[ly][lx] + chunk_info.grass_height_map.heights[ly][lx];
        slice.blocks[ly][lx] = lz < height1 ? Block::STONE :
                               lz < height2 ? Block::GRASS :
                                              Block::NONE;
      }
    chunk_data.slices.push_back(slice);
  }

  // 2: Carve out caves based off worms
  int        radius  = std::ceil(CAVE_WORM_SEGMENT_MAX * CAVE_WORM_STEP / CHUNK_WIDTH);
  glm::ivec2 corner1 = chunk_position - glm::ivec2(radius, radius);
  glm::ivec2 corner2 = chunk_position + glm::ivec2(radius, radius);
  for(int cy = corner1.y; cy <= corner2.y; ++cy)
    for(int cx = corner1.x; cx <= corner2.x; ++cx)
    {
      glm::ivec2 neighbour_chunk_position = glm::ivec2(cx, cy);

      auto it = chunk_infos.find(neighbour_chunk_position);
      assert(it != chunk_infos.end());

      const ChunkInfo& chunk_info = it->second;
      for(const Worm& worm : chunk_info.worms)
        for(const Worm::Node& node : worm.nodes)
          explode(chunk_data, global_to_local(node.center, chunk_position), node.radius);
    }

  return chunk_data;
}

/**************
 * Chunk Mesh *
 **************/
static constexpr glm::ivec3 DIRECTIONS[] = {
  {-1, 0, 0},
  { 1, 0, 0},
  {0, -1, 0},
  {0,  1, 0},
  {0, 0, -1},
  {0, 0,  1},
};

static Mesh generate_chunk_mesh(glm::ivec2 chunk_position, const ChunkData& chunk_data)
{
  struct Vertex
  {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
  };

  std::vector<uint32_t> indices;
  std::vector<Vertex>   vertices;
  for(int lz=0; lz<chunk_data.slices.size(); ++lz)
    for(int ly=0; ly<CHUNK_WIDTH; ++ly)
      for(int lx=0; lx<CHUNK_WIDTH; ++lx)
      {
        glm::ivec3 position  = { lx, ly, lz };
        Block      block     = get_block(chunk_data, position);
        if(!block.presence)
          continue;

        for(glm::ivec3 direction : DIRECTIONS)
        {
          glm::ivec3 neighbour_position = position + direction;
          Block      neighbour_block    = get_block(chunk_data, neighbour_position);
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

          glm::vec3 color;
          switch(block.id)
          {
          case Block::ID_STONE: color = glm::vec3(0.7, 0.7, 0.7); break;
          case Block::ID_GRASS: color = glm::vec3(0.2, 1.0, 0.2); break;
          default: color = glm::vec3(1.0, 1.0, 1.0); break;
          }

          vertices.push_back(Vertex{ .position = center + ( - 0.5f * glm::vec3(right) - 0.5f * glm::vec3(up)), .normal = direction, .color = color });
          vertices.push_back(Vertex{ .position = center + ( + 0.5f * glm::vec3(right) - 0.5f * glm::vec3(up)), .normal = direction, .color = color });
          vertices.push_back(Vertex{ .position = center + ( - 0.5f * glm::vec3(right) + 0.5f * glm::vec3(up)), .normal = direction, .color = color });
          vertices.push_back(Vertex{ .position = center + ( + 0.5f * glm::vec3(right) + 0.5f * glm::vec3(up)), .normal = direction, .color = color });
          // NOTE: Brackets added so that it is possible for the compiler to do constant folding if loop is unrolled, not that it would actually do it.
        }
      }

  MeshLayout layout{
    .index_type = IndexType::UNSIGNED_INT,
    .stride = sizeof(Vertex),
    .attributes = {
      { .type = AttributeType::FLOAT3, .offset = offsetof(Vertex, position), },
      { .type = AttributeType::FLOAT3, .offset = offsetof(Vertex, normal),   },
      { .type = AttributeType::FLOAT3, .offset = offsetof(Vertex, color),    },
    },
  };

  return Mesh(layout,
    as_bytes(indices),
    as_bytes(vertices)
  );
}

/**********
 * Entity *
 **********/
static void entity_apply_force(Entity& entity, glm::vec3 force, float dt)
{
  entity.velocity += dt * force;
}

static void entity_update_physics(Entity& entity, float dt)
{
  entity_apply_force(entity, -FRICTION * entity.velocity, dt);
  entity_apply_force(entity, GRAVITY,                     dt);
  entity.position += dt * entity.velocity;
}

static glm::vec3 aabb_collide(glm::vec3 position1, glm::vec3 dimension1, glm::vec3 position2, glm::vec3 dimension2)
{
  glm::vec3 point1 = position2 - (position1 + dimension1);
  glm::vec3 point2 = (position2 + dimension2) - position1;
  float x = std::abs(point1.x) < std::abs(point2.x) ? point1.x : point2.x;
  float y = std::abs(point1.y) < std::abs(point2.y) ? point1.y : point2.y;
  float z = std::abs(point1.z) < std::abs(point2.z) ? point1.z : point2.z;
  return glm::vec3(x, y, z);
}

static std::vector<glm::vec3> entity_collide(const Entity& entity, const std::unordered_map<glm::ivec2, ChunkData>& chunk_datas)
{
  std::vector<glm::vec3> collisions;

  glm::ivec3 corner1 = glm::floor(entity.position);
  glm::ivec3 corner2 = -glm::floor(-(entity.position + entity.bounding_box))-1.0f;
  for(int z = corner1.z; z<=corner2.z; ++z)
    for(int y = corner1.y; y<=corner2.y; ++y)
      for(int x = corner1.x; x<=corner2.x; ++x)
      {
        glm::ivec3 block_global_position = glm::ivec3(x, y, z);
        glm::ivec3 block_local_position = {
          modulo(block_global_position.x, CHUNK_WIDTH),
          modulo(block_global_position.y, CHUNK_WIDTH),
          block_global_position.z
        };
        glm::ivec2 chunk_position = {
          (block_global_position.x - block_local_position.x) / CHUNK_WIDTH,
          (block_global_position.y - block_local_position.y) / CHUNK_WIDTH,
        };

        auto it = chunk_datas.find(chunk_position);
        if(it == chunk_datas.end())
          continue;

        const ChunkData& chunk_data = it->second;
        Block block = get_block(chunk_data, block_local_position);
        if(block.presence)
        {
          glm::vec3 collision = aabb_collide(entity.position, entity.bounding_box, block_global_position, glm::vec3(1.0f, 1.0f, 1.0f));
          spdlog::info("Entity collision {}, {}, {} with block {}, {}, {}",
            collision.x, collision.y, collision.z,
            block_global_position.x, block_global_position.y, block_global_position.z
          );
          collisions.push_back(collision);
        }
      }

  spdlog::info("Entity colliding = {}", !collisions.empty());
  return collisions;
}

void entity_resolve_collisions(Entity& entity, const std::unordered_map<glm::ivec2, ChunkData>& chunk_datas)
{
  for(int i=0; i<20; ++i)
  {
    spdlog::warn("Interation {}", i);
    spdlog::info("Entity position = {}, {}, {}", entity.position.x, entity.position.y, entity.position.z);
    spdlog::info("Entity velocity = {}, {}, {}", entity.velocity.x, entity.velocity.y, entity.velocity.z);

    std::vector<glm::vec3> collisions = entity_collide(entity, chunk_datas);
    if(collisions.empty())
    {
      spdlog::info("Entity final position = {}, {}, {}", entity.position.x, entity.position.y, entity.position.z);
      spdlog::info("Entity final velocity = {}, {}, {}", entity.velocity.x, entity.velocity.y, entity.velocity.z);
      spdlog::info("Collision successfully resolved");
      return;
    }

    float                    min = std::numeric_limits<float>::infinity();
    std::optional<glm::vec3> resolution;

    for(glm::vec3 collision : collisions)
      for(glm::vec3 direction : DIRECTIONS)
        if(float length = glm::dot(collision, direction); 0.0f < length && length < min)
        {
          min        = length;
          resolution = length * direction;
        }

    if(resolution)
    {
      spdlog::info("Resolving collision by {}, {}, {}", resolution->x, resolution->y, resolution->z);
      entity.position += *resolution;
      if(resolution->x != 0.0f) entity.velocity.x = 0.0f;
      if(resolution->y != 0.0f) entity.velocity.y = 0.0f;
      if(resolution->z != 0.0f) entity.velocity.z = 0.0f;
    }

  }
  assert(false && "Collision resolution failed"); // TODO: Consider restoring the original position to avoid faild collision resolution ot have sling-shot effect
}

/*********
 * World *
 *********/
World::World(std::size_t seed) :
  m_camera{
    .position = glm::vec3(-5.0f, -5.0f,  50.0f),
    .aspect = 1024.0f / 720.0f,
    .yaw    = 45.0f,
    .pitch  = -45.0f,
    .fov    = 45.0f,
  },
  m_lights{{
    .position = { 0.0f, 0.0f, 30.0f}, // position
    .ambient  = { 0.2f, 0.2f, 0.2f },  // ambient
    .diffuse  = { 0.5f, 0.5f, 0.5f },  // diffuse
  }},
  m_seed(seed),
  m_player{
    .position     = glm::vec3(0.0f, 0.0f, 50.0f),
    .velocity     = glm::vec3(0.0f, 0.0f, 0.0f),
    .bounding_box = glm::vec3(0.9f, 0.9f, 1.9f),
  },
  m_program(gl::compile_program("assets/chunk.vert", "assets/chunk.frag"))
{
  unsigned count = std::thread::hardware_concurrency();
  for(unsigned i=0; i<count; ++i)
    m_workers.emplace_back(std::bind(&World::work, this, std::placeholders::_1));
}

void World::handle_event(SDL_Event event)
{
  switch(event.type)
  {
    case SDL_MOUSEMOTION:
      m_camera.rotate(-event.motion.xrel, -event.motion.yrel);
      break;
    case SDL_MOUSEWHEEL:
      m_camera.zoom(-event.wheel.y);
      break;
  }
}

void World::update(float dt)
{
  // 1: Camera Movement
  glm::vec3 translation = glm::vec3(0.0f);

  const Uint8 *keys = SDL_GetKeyboardState(nullptr);
  if(keys[SDL_SCANCODE_SPACE])  translation.z += 1.0f;
  if(keys[SDL_SCANCODE_LSHIFT]) translation.z -= 1.0f;
  if(keys[SDL_SCANCODE_W])      translation.y += 1.0f;
  if(keys[SDL_SCANCODE_S])      translation.y -= 1.0f;
  if(keys[SDL_SCANCODE_D])      translation.x += 1.0f;
  if(keys[SDL_SCANCODE_A])      translation.x -= 1.0f;
  if(glm::length(translation) != 0.0f)
  {
    translation = glm::normalize(translation);
    translation *= dt;
    m_player.velocity += translation * 10.0f;
  }

  glm::ivec2 center_chunk_position = {
    std::floor(m_camera.position.x / CHUNK_WIDTH),
    std::floor(m_camera.position.y / CHUNK_WIDTH),
  };

  // 2: Loading
  {
    std::lock_guard lk(m_mutex);
    for(int cy = center_chunk_position.y - CHUNK_LOAD_RADIUS; cy <= center_chunk_position.y + CHUNK_LOAD_RADIUS; ++cy)
      for(int cx = center_chunk_position.x - CHUNK_LOAD_RADIUS; cx <= center_chunk_position.x + CHUNK_LOAD_RADIUS; ++cx)
      {
        glm::ivec2 chunk_position(cx, cy);
        try_load_mesh(chunk_position);
      }
  }
  m_cv.notify_all();

  // 3: Entity Update
  {
    std::lock_guard lk(m_mutex);
    entity_update_physics(m_player, dt);
    entity_resolve_collisions(m_player, m_chunk_datas);
  }
  m_camera.position = m_player.position + glm::vec3(0.5f, 0.5f, 1.5f);
}

void World::render()
{
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 view       = m_camera.view();
  glm::mat4 projection = m_camera.projection();

  glUseProgram(m_program);
  {
    glUniform3fv(glGetUniformLocation(m_program, "viewPos"),        1, glm::value_ptr(m_camera.position));
    glUniform3fv(glGetUniformLocation(m_program, "light.pos"),      1, glm::value_ptr(m_lights.at(0).position));
    glUniform3fv(glGetUniformLocation(m_program, "light.ambient"),  1, glm::value_ptr(m_lights.at(0).ambient));
    glUniform3fv(glGetUniformLocation(m_program, "light.diffuse"),  1, glm::value_ptr(m_lights.at(0).diffuse));

    std::shared_lock shared_lk(m_mutex);
    for(const auto& [chunk_position, chunk_mesh] : m_chunk_meshes)
    {
      glm::mat4 model  = glm::translate(glm::mat4(1.0f), glm::vec3( CHUNK_WIDTH * chunk_position.x, CHUNK_WIDTH * chunk_position.y, 0.0f));
      glm::mat4 normal = glm::transpose(glm::inverse(model));
      glm::mat4 MVP    = projection * view * model;

      glUniformMatrix4fv(glGetUniformLocation(m_program, "MVP"),    1, GL_FALSE, glm::value_ptr(MVP));
      glUniformMatrix4fv(glGetUniformLocation(m_program, "model"),  1, GL_FALSE, glm::value_ptr(model));
      glUniformMatrix4fv(glGetUniformLocation(m_program, "normal"), 1, GL_FALSE, glm::value_ptr(normal));

      chunk_mesh.draw();
    }
  }
}

bool World::try_load_info(glm::ivec2 chunk_position)
{
  if(m_chunk_infos.contains(chunk_position))         return true;
  if(m_pending_chunk_infos.contains(chunk_position)) return false;
  if(m_loading_chunk_infos.contains(chunk_position)) return false;

  m_pending_chunk_infos.insert(chunk_position);
  return false;
}

bool World::try_load_data(glm::ivec2 chunk_position)
{
  if(m_chunk_datas.contains(chunk_position))         return true;
  if(m_pending_chunk_datas.contains(chunk_position)) return false;
  if(m_loading_chunk_datas.contains(chunk_position)) return false;

  bool success = true;

  int        radius  = std::ceil(CAVE_WORM_SEGMENT_MAX * CAVE_WORM_STEP / CHUNK_WIDTH);
  glm::ivec2 corner1 = chunk_position - glm::ivec2(radius, radius);
  glm::ivec2 corner2 = chunk_position + glm::ivec2(radius, radius);
  for(int cy = corner1.y; cy <= corner2.y; ++cy)
    for(int cx = corner1.x; cx <= corner2.x; ++cx)
    {
      glm::ivec2 neighbour_chunk_position = glm::ivec2(cx, cy);
      if(!try_load_info(neighbour_chunk_position))
        success = false;
    }

  if(success) m_pending_chunk_datas.insert(chunk_position);
  return false;
}

bool World::try_load_mesh(glm::ivec2 chunk_position)
{
  if(m_chunk_meshes.contains(chunk_position))         return true;
  if(m_pending_chunk_meshes.contains(chunk_position)) return false;
  if(m_loading_chunk_meshes.contains(chunk_position)) return false;

  bool success = try_load_data(chunk_position);

  if(success) m_pending_chunk_meshes.insert(chunk_position);
  return false;
}

void World::work(std::stop_token stoken)
{
  std::unique_lock lk(m_mutex);
  for(;;)
  {
    m_cv.wait(lk, stoken, [this]() {
      return !m_pending_chunk_infos.empty()
          || !m_pending_chunk_datas.empty()
          || !m_pending_chunk_meshes.empty();
    });
    if(stoken.stop_requested())
      return;

    for(;;)
    {
      if(stoken.stop_requested())
        return;
      else if(!m_pending_chunk_meshes.empty())
      {
        glm::ivec2 chunk_position = *m_pending_chunk_meshes.begin();
        m_pending_chunk_meshes.erase(m_pending_chunk_meshes.begin());
        m_loading_chunk_meshes.insert(chunk_position);
        spdlog::info("Generating chunk mesh at {}, {}", chunk_position.x, chunk_position.y);

        lk.unlock();

        std::shared_lock shared_lk(m_mutex);
        Mesh chunk_mesh = generate_chunk_mesh(chunk_position, m_chunk_datas.at(chunk_position));
        shared_lk.unlock();

        lk.lock();

        m_loading_chunk_datas.erase(chunk_position);
        m_chunk_meshes.emplace(chunk_position, std::move(chunk_mesh));
      }
      else if(!m_pending_chunk_datas.empty())
      {
        glm::ivec2 chunk_position = *m_pending_chunk_datas.begin();
        m_pending_chunk_datas.erase(m_pending_chunk_datas.begin());
        m_loading_chunk_datas.insert(chunk_position);
        spdlog::info("Generating chunk data at {}, {}", chunk_position.x, chunk_position.y);

        lk.unlock();

        std::shared_lock shared_lk(m_mutex);
        ChunkData chunk_data = generate_chunk_data(chunk_position, m_chunk_infos);
        shared_lk.unlock();

        lk.lock();

        m_loading_chunk_datas.erase(chunk_position);
        m_chunk_datas.emplace(chunk_position, std::move(chunk_data));
      }
      else if(!m_pending_chunk_infos.empty())
      {
        glm::ivec2 chunk_position = *m_pending_chunk_infos.begin();
        m_pending_chunk_infos.erase(m_pending_chunk_infos.begin());
        m_loading_chunk_infos.insert(chunk_position);
        spdlog::info("Generating chunk info at {}, {}", chunk_position.x, chunk_position.y);

        lk.unlock();
        ChunkInfo chunk_info = generate_chunk_info(chunk_position, m_seed);
        lk.lock();

        m_loading_chunk_infos.erase(chunk_position);
        m_chunk_infos.emplace(chunk_position, std::move(chunk_info));
      }
      else
        break;
    }
  }
}
