#include <system/physics.hpp>

#include <world.hpp>
#include <directions.hpp>

#include <spdlog/spdlog.h>

#include <optional>
#include <unordered_set>
#include <algorithm>

class PhysicsSystem : public System
{
private:
  static constexpr float FRICTION = 0.5f;
  static constexpr float GRAVITY  = 5.0f;
  static constexpr int MAX_COLLISION_ITERATION = 5;

private:
  struct AABB
  {
    glm::vec3 position;
    glm::vec3 dimension;
  };

  struct SweptAABBResult
  {
    float t_in;  glm::vec3 normal_in;
    float t_out; glm::vec3 normal_out;
  };

  static std::optional<SweptAABBResult> swept_aabb(AABB box1, AABB box2, glm::vec3 direction)
  {
    glm::vec3 ds1 = box2.position - (box1.position + box1.dimension); // Distance to enter
    glm::vec3 ds2 = (box2.position + box2.dimension) - box1.position; // Distance to leave

    SweptAABBResult result = {
      .t_in  = -std::numeric_limits<float>::infinity(),
      .t_out = +std::numeric_limits<float>::infinity(),
    };
    for(int i=0; i<3; ++i)
    {
      float d1 = ds1[i];
      float d2 = ds2[i];
      if(direction[i] != 0.0f)
      {
        float t1 = d1 / direction[i];
        float t2 = d2 / direction[i];
        if(t1 > t2)
          std::swap(t1, t2);

        if(result.t_in < t1)
        {
          result.t_in         = t1;
          result.normal_in    = {};
          result.normal_in[i] = direction[i] > 0.0f ? -1 : 1;
        }

        if(result.t_out > t2)
        {
          result.t_out         = t2;
          result.normal_out    = {};
          result.normal_out[i] = direction[i] > 0.0f ? -1 : 1;
        }
      }
      else
        if(d1 * d2 > 0.0f)
          return std::nullopt;
    }
    return result;
  }

  static std::optional<SweptAABBResult> swept_aabb_fold(SweptAABBResult lhs, SweptAABBResult rhs)
  {
    SweptAABBResult result = {};
    if(lhs.t_in  < rhs.t_in)  { result.t_in  = lhs.t_in;  result.normal_in  = lhs.normal_in;  } else { result.t_in  = rhs.t_in;  result.normal_in  = rhs.normal_in; }
    if(lhs.t_out < rhs.t_out) { result.t_out = lhs.t_out; result.normal_out = lhs.normal_out; } else { result.t_out = rhs.t_out; result.normal_out = rhs.normal_out; }
    return result;
  }

  static std::optional<SweptAABBResult> swept_aabb_fold(std::optional<SweptAABBResult> lhs, std::optional<SweptAABBResult> rhs)
  {
    if(lhs && rhs) return swept_aabb_fold(*lhs, *rhs);
    if(lhs && !rhs) return lhs;
    if(rhs && !lhs) return rhs;
    return std::nullopt;
  }

private:
  static void entity_update_physics(Entity& entity, float dt)
  {
    entity.apply_force(-FRICTION * entity.velocity,             dt);
    entity.apply_force(-GRAVITY  * glm::vec3(0.0f, 0.0f, 1.0f), dt);
    entity.transform.position += dt * entity.velocity;
  }

  static void entity_resolve_collisions(Entity& entity, const Dimension& dimension)
  {
    entity.collided = false;

    glm::vec3 original_position = entity.transform.position;
    glm::vec3 original_velocity = entity.velocity;

    for(int i=0; i<MAX_COLLISION_ITERATION; ++i)
    {
      std::optional<SweptAABBResult> result;

      glm::ivec3 corner1 = glm::floor(entity.transform.position);
      glm::ivec3 corner2 = -glm::floor(-(entity.transform.position + entity.bounding_box))-1.0f;
      for(int z = corner1.z; z<=corner2.z; ++z)
        for(int y = corner1.y; y<=corner2.y; ++y)
          for(int x = corner1.x; x<=corner2.x; ++x)
          {
            glm::ivec3   position = glm::ivec3(x, y, z);
            const Block* block    = dimension.get_block(position);
            if(block && block->id != Block::ID_NONE)
            {
              AABB entity_aabb = { .position = entity.transform.position, .dimension = entity.bounding_box, };
              AABB block_aabb  = { .position = position,                  .dimension = glm::vec3(1.0f),     };
              glm::vec3 direction = entity.velocity;
              result = swept_aabb_fold(result, swept_aabb(entity_aabb, block_aabb, direction));
            }
          }

      if(result && result->t_in < result->t_out && result->t_in * result->t_out < 0.0f)
      {
        entity.collided = true;
        entity.transform.position += glm::dot(entity.velocity, result->normal_in) * result->normal_in * result->t_in; // Backtrack in normal direction
        entity.velocity           -= glm::dot(entity.velocity, result->normal_in) * result->normal_in;                // Cancel velocity in normal direction
      }
      else
        return;
    }

    spdlog::warn("Collision resolution failed");
    entity.transform.position = original_position;
    entity.velocity           = original_velocity;
  }

  void on_update(World& world, float dt) override
  {
    entity_update_physics(world.player, dt);
    entity_resolve_collisions(world.player, world.dimension);
  }
};

std::unique_ptr<System> create_physics_system()
{
  return std::make_unique<PhysicsSystem>();
}

