#include <system/physics.hpp>

#include <types/world.hpp>
#include <types/directions.hpp>

#include <spdlog/spdlog.h>

#include <unordered_set>

static constexpr float FRICTION = 0.5f;
static constexpr float GRAVITY  = 5.0f;
static constexpr int MAX_COLLISION_ITERATION = 5;

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

static std::vector<glm::vec3> entity_collide(const Entity& entity, const Dimension& dimension)
{
  std::vector<glm::vec3> collisions;

  glm::ivec3 corner1 = glm::floor(entity.transform.position);
  glm::ivec3 corner2 = -glm::floor(-(entity.transform.position + entity.bounding_box))-1.0f;
  for(int z = corner1.z; z<=corner2.z; ++z)
    for(int y = corner1.y; y<=corner2.y; ++y)
      for(int x = corner1.x; x<=corner2.x; ++x)
      {
        glm::ivec3 position = glm::ivec3(x, y, z);
        Block block = dimension.get_block(position).value_or(Block{.presence = false});
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

static void entity_resolve_collisions(Entity& entity, const Dimension& dimension)
{
  glm::vec3 original_position = entity.transform.position;
  glm::vec3 original_velocity = entity.velocity;

  for(int i=0; i<MAX_COLLISION_ITERATION; ++i)
  {
    std::vector<glm::vec3> collisions = entity_collide(entity, dimension);
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


class PhysicsSystemImpl : public PhysicsSystem
{
private:
  void update(World& world, float dt) override
  {
    entity_update_physics(world.player, dt);
    entity_resolve_collisions(world.player, world.dimension);
  }
};

std::unique_ptr<PhysicsSystem> PhysicsSystem::create()
{
  return std::make_unique<PhysicsSystemImpl>();
}

