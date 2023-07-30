#include <world.hpp>

#include <glm/gtx/norm.hpp>

#include <spdlog/spdlog.h>

/*************
 * Constants *
 *************/
static constexpr float FRICTION = 0.5f;
static constexpr float GRAVITY  = 5.0f;
static constexpr int MAX_COLLISION_ITERATION = 5;

static constexpr float ROTATION_SPEED = 0.1f;

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

static std::vector<glm::vec3> entity_collide(const Entity& entity, const ChunkManager& chunk_manager)
{
  std::vector<glm::vec3> collisions;

  glm::ivec3 corner1 = glm::floor(entity.transform.position);
  glm::ivec3 corner2 = -glm::floor(-(entity.transform.position + entity.bounding_box))-1.0f;
  for(int z = corner1.z; z<=corner2.z; ++z)
    for(int y = corner1.y; y<=corner2.y; ++y)
      for(int x = corner1.x; x<=corner2.x; ++x)
      {
        glm::ivec3 position = glm::ivec3(x, y, z);
        Block block = chunk_manager.get_block(position).value_or(Block{.presence = false});
        if(block.presence)
        {
          glm::vec3 collision = aabb_collide(entity.transform.position, entity.bounding_box, position, glm::vec3(1.0f, 1.0f, 1.0f));
          spdlog::info("Entity collision {}, {}, {} with block {}, {}, {}",
            collision.x, collision.y, collision.z,
            position.x, position.y, position.z
          );
          collisions.push_back(collision);
        }
      }

  spdlog::info("Entity colliding = {}", !collisions.empty());
  return collisions;
}

void entity_resolve_collisions(Entity& entity, const ChunkManager& chunk_manager)
{
  glm::vec3 original_position = entity.transform.position;
  glm::vec3 original_velocity = entity.velocity;

  for(int i=0; i<MAX_COLLISION_ITERATION; ++i)
  {
    std::vector<glm::vec3> collisions = entity_collide(entity, chunk_manager);
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
  m_player{
    .transform = {
      .position = glm::vec3(0.0f, 0.0f, 50.0f),
      .rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
    },
    .velocity     = glm::vec3(0.0f, 0.0f, 0.0f),
    .bounding_box = glm::vec3(0.9f, 0.9f, 1.9f),
  },
  m_chunk_manager(seed)
{}

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

  // 2: Lazy chunk loading
  glm::ivec2 center = {
    std::floor(m_player.transform.position.x / CHUNK_WIDTH),
    std::floor(m_player.transform.position.y / CHUNK_WIDTH),
  };
  m_chunk_manager.load(center, CHUNK_LOAD_RADIUS);

  // 3: Update
  m_chunk_manager.update();

  // 4: Entity Update
  entity_update_physics(m_player, dt);
  entity_resolve_collisions(m_player, m_chunk_manager);

  // 5: Camera update
  m_camera.transform           = m_player.transform;
  m_camera.transform.position += glm::vec3(0.5f, 0.5f, 1.5f);
}

void World::render()
{
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  m_chunk_manager.render(m_camera);
}


