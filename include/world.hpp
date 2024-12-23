#pragma once

#include <transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_set>
#include <optional>
#include <memory>

#include <cstddef>

static constexpr int CHUNK_WIDTH  = 16;
static constexpr int CHUNK_HEIGHT = 256;

static constexpr std::uint32_t BLOCK_ID_STONE = 0;
static constexpr std::uint32_t BLOCK_ID_GRASS = 1;
static constexpr std::uint32_t BLOCK_ID_NONE  = 2;

struct AABB
{
  glm::vec3 position;
  glm::vec3 dimension;
};

struct Entity
{
  std::uint16_t id;
  Transform     transform;
  glm::vec3     velocity;

  glm::vec3 dimension;
  float     eye;

  bool collided;
  bool grounded;
};

struct Player
{
  size_t entity_id;

  bool key_space : 1;
  bool key_w     : 1;
  bool key_a     : 1;
  bool key_s     : 1;
  bool key_d     : 1;

  bool mouse_button_left  : 1;
  bool mouse_button_right : 1;

  double cursor_motion_x;
  double cursor_motion_y;

  float cooldown;
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

  mutable bool                           mesh_invalidated;
};

struct World
{
  std::unordered_map<glm::ivec2, Chunk> chunks;
  std::vector<Entity>                   entities;
  std::vector<Player>                   players;
};

/**********
 * Entity *
 **********/
AABB entity_get_aabb(const Entity& entity);

void entity_apply_impulse (Entity& entity, glm::vec3 force);
void entity_apply_force   (Entity& entity, glm::vec3 force, float dt);
void entity_apply_force   (Entity& entity, glm::vec3 force, float dt, float max);

/******************
 * Block Accessor *
 ******************/
      Block* get_block(      Chunk& chunk, glm::ivec3 position);
const Block* get_block(const Chunk& chunk, glm::ivec3 position);

      Block* get_block(      World& world, glm::ivec3 position);
const Block* get_block(const World& world, glm::ivec3 position);

/**************************
 * Invalidate them ALL!!! *
 **************************/
void invalidate_mesh (Chunk& chunk);

void invalidate_mesh(World& world, glm::ivec3 position);

World load_world(std::string_view path);
