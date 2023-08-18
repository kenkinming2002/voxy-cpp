#pragma once

#include <transform.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_set>

#include <optional>

#include <cstddef>

static constexpr int CHUNK_WIDTH  = 16;
static constexpr int CHUNK_HEIGHT = 256;

static constexpr std::uint32_t BLOCK_ID_STONE = 0;
static constexpr std::uint32_t BLOCK_ID_GRASS = 1;
static constexpr std::uint32_t BLOCK_ID_NONE  = 2;

struct Entity
{
  Transform transform;

  glm::vec3 velocity;
  glm::vec3 bounding_box;

  glm::vec3 eye_offset;

  bool collided;
  bool grounded;
};

struct Block
{
  std::uint32_t id            : 23;
  std::uint32_t sky           : 1;
  std::uint32_t light_level   : 4;
  std::uint32_t destroy_level : 4;
};

struct Chunk
{
  Block blocks[CHUNK_HEIGHT][CHUNK_WIDTH][CHUNK_WIDTH];

  mutable bool   mesh_invalidated_major;
  mutable bool   mesh_invalidated_minor;
  mutable double last_remash_time;

  std::unordered_set<glm::ivec3> pending_lighting_updates;
};

struct Dimension
{
  std::unordered_map<glm::ivec2, Chunk> chunks;
};

struct World
{
  Entity    player;
  Dimension dimension;

  std::optional<glm::ivec3> placement;
  std::optional<glm::ivec3> selection;
};

/**********
 * Entity *
 **********/
void entity_apply_force  (Entity& entity, glm::vec3 force, float dt);
void entity_apply_impulse(Entity& entity, glm::vec3 force);

/******************
 * Block Accessor *
 ******************/
      Block* get_block(      Chunk& chunk, glm::ivec3 position);
const Block* get_block(const Chunk& chunk, glm::ivec3 position);

      Block* get_block(      Dimension& dimension, glm::ivec3 position);
const Block* get_block(const Dimension& dimension, glm::ivec3 position);

      Block* get_block(      World& world, glm::ivec3 position);
const Block* get_block(const World& world, glm::ivec3 position);

/************************
 * Explode the World!!! *
 ************************/
void explode(Chunk& chunk, glm::vec3 center, float radius);

/**************************
 * Invalidate them ALL!!! *
 **************************/
void invalidate_mesh_major(Chunk& chunk);
void invalidate_mesh_minor(Chunk& chunk);
void invalidate_light     (Chunk& chunk, glm::ivec3 position);

void invalidate_mesh_major(Dimension& dimension, glm::ivec3 position);
void invalidate_mesh_minor(Dimension& dimension, glm::ivec3 position);
void invalidate_light     (Dimension& dimension, glm::ivec3 position);

void invalidate_mesh_major(World& world, glm::ivec3 position);
void invalidate_mesh_minor(World& world, glm::ivec3 position);
void invalidate_light     (World& world, glm::ivec3 position);

