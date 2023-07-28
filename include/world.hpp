#ifndef WORLD_HPP
#define WORLD_HPP

#include <camera.hpp>
#include <light.hpp>
#include <mesh.hpp>

#include <gl.hpp>

#include <SDL.h>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <mutex>
#include <shared_mutex>
#include <condition_variable>

#include <thread>

#include <unordered_set>
#include <unordered_map>

#include <variant>
#include <utility>

#include <stddef.h>
#include <stdint.h>

static constexpr int CHUNK_WIDTH = 16;

/*************
 * ChunkInfo *
 *************/
struct HeightMap
{
  float heights[CHUNK_WIDTH][CHUNK_WIDTH];
};

struct Worm
{
  struct Node
  {
    glm::vec3 center;
    float     radius;
  };
  std::vector<Node> nodes;
};

struct ChunkInfo
{
  HeightMap         stone_height_map;
  HeightMap         grass_height_map;
  std::vector<Worm> worms;
};

/*************
 * ChunkData *
 *************/
struct Block
{
  static constexpr uint32_t ID_NONE  = 0;
  static constexpr uint32_t ID_STONE = 1;
  static constexpr uint32_t ID_GRASS = 2;

  static const Block NONE;
  static const Block STONE;
  static const Block GRASS;

  uint32_t id       : 31;
  uint32_t presence : 1;
};

inline const Block Block::NONE  = { .id = Block::ID_NONE,  .presence = 0, };
inline const Block Block::STONE = { .id = Block::ID_STONE, .presence = 1, };
inline const Block Block::GRASS = { .id = Block::ID_GRASS, .presence = 1, };

struct ChunkData
{
  struct Slice { Block blocks[CHUNK_WIDTH][CHUNK_WIDTH]; };
  std::vector<Slice> slices;
};

/**********
 * Entity *
 **********/
struct Entity
{
  glm::vec3 position;
  glm::vec3 velocity;
  glm::vec3 bounding_box;
};

/*********
 * World *
 *********/
struct World
{
public:
  World(std::size_t seed);

public:
  void handle_event(SDL_Event event);
  void update(float dt);
  void render();

private:
  // Try to load info/data/mesh
  //
  // @precondition m_mutex is held
  // @return       true if info/data/mesh has already been loaded, false otherwise
  bool try_load_info(glm::ivec2 chunk_position);
  bool try_load_data(glm::ivec2 chunk_position);
  bool try_load_mesh(glm::ivec2 chunk_position);

  void work(std::stop_token stoken);

private:
  Camera             m_camera;
  std::vector<Light> m_lights;

private:
  std::size_t m_seed;

  std::shared_mutex           m_mutex;
  std::condition_variable_any m_cv;

  std::unordered_set<glm::ivec2> m_pending_chunk_infos;
  std::unordered_set<glm::ivec2> m_pending_chunk_datas;
  std::unordered_set<glm::ivec2> m_pending_chunk_meshes;

  std::unordered_set<glm::ivec2> m_loading_chunk_infos;
  std::unordered_set<glm::ivec2> m_loading_chunk_datas;
  std::unordered_set<glm::ivec2> m_loading_chunk_meshes;

  std::unordered_map<glm::ivec2, ChunkInfo> m_chunk_infos;
  std::unordered_map<glm::ivec2, ChunkData> m_chunk_datas;
  std::unordered_map<glm::ivec2, Mesh>      m_chunk_meshes;

private:
  Entity m_player;

private:
  gl::Program m_program;

private:
  std::vector<std::jthread> m_workers;
};

#endif // WORLD_HPP
