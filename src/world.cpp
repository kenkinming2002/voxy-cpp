#include <world.hpp>

#include <chunk_coords.hpp>
#include <chunk_info.hpp>
#include <chunk_data.hpp>
#include <chunk_mesh.hpp>

#include <perlin.hpp>

#include <glm/gtx/norm.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <spdlog/spdlog.h>

#include <random>

/*************
 * Constants *
 *************/
static constexpr float FRICTION = 0.5f;
static constexpr float GRAVITY  = 5.0f;
static constexpr int MAX_COLLISION_ITERATION = 5;

static constexpr float ROTATION_SPEED = 0.1f;

/*************
 * Utilities *
 *************/
static int modulo(int a, int b)
{
  int value = a % b;
  if(value < 0)
    value += b;

  assert(0 <= value);
  assert(value < b);
  return value;
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
  entity_apply_force(entity, -FRICTION * entity.velocity,             dt);
  entity_apply_force(entity, -GRAVITY  * glm::vec3(0.0f, 0.0f, 1.0f), dt);
  entity.transform.position += dt * entity.velocity;
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

  glm::ivec3 corner1 = glm::floor(entity.transform.position);
  glm::ivec3 corner2 = -glm::floor(-(entity.transform.position + entity.bounding_box))-1.0f;
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
          glm::vec3 collision = aabb_collide(entity.transform.position, entity.bounding_box, block_global_position, glm::vec3(1.0f, 1.0f, 1.0f));
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
  glm::vec3 original_position = entity.transform.position;
  glm::vec3 original_velocity = entity.velocity;

  for(int i=0; i<MAX_COLLISION_ITERATION; ++i)
  {
    std::vector<glm::vec3> collisions = entity_collide(entity, chunk_datas);
    if(collisions.empty())
      return;

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
      entity.transform.position += *resolution;
      if(resolution->x != 0.0f) entity.velocity.x = 0.0f;
      if(resolution->y != 0.0f) entity.velocity.y = 0.0f;
      if(resolution->z != 0.0f) entity.velocity.z = 0.0f;
    }

  }
  spdlog::warn("Failed to resolve collision");
  entity.transform.position = original_position;
  entity.velocity = original_velocity;
}

/*********
 * World *
 *********/
World::World(std::size_t seed) :
  m_camera{
    .aspect = 1024.0f / 720.0f,
    .fovy   = 45.0f,
  },
  m_lights{{
    .position = { 0.0f, 0.0f, 30.0f}, // position
    .ambient  = { 0.2f, 0.2f, 0.2f },  // ambient
    .diffuse  = { 0.5f, 0.5f, 0.5f },  // diffuse
  }},
  m_seed(seed),
  m_player{
    .transform = {
      .position = glm::vec3(0.0f, 0.0f, 50.0f),
      .rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
    },
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
      m_player.transform = m_player.transform.rotate(glm::vec3(0.0f,
        -event.motion.yrel * ROTATION_SPEED,
        -event.motion.xrel * ROTATION_SPEED
      ));
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
    translation = m_player.transform.gocal_to_global(translation);
    translation = glm::normalize(translation);
    translation *= dt;
    m_player.velocity += translation * 10.0f;
  }

  glm::ivec2 center_chunk_position = {
    std::floor(m_player.transform.position.x / CHUNK_WIDTH),
    std::floor(m_player.transform.position.y / CHUNK_WIDTH),
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


  m_camera.transform           = m_player.transform;
  m_camera.transform.position += glm::vec3(0.5f, 0.5f, 1.5f);
}

void World::render()
{
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 view       = m_camera.view();
  glm::mat4 projection = m_camera.projection();

  glUseProgram(m_program);
  {
    glUniform3fv(glGetUniformLocation(m_program, "viewPos"),        1, glm::value_ptr(m_camera.transform.position));
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

