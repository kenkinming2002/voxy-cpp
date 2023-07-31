#include <world.hpp>

#include <chunk_coords.hpp>

#include <glm/gtx/norm.hpp>

#include <spdlog/spdlog.h>

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
  m_dimension {
    .blocks_texture_array = TextureArray({
        "assets/stone.png",
        "assets/grass_bottom.png",
        "assets/grass_side.png",
        "assets/grass_top.png",
    }),
    .block_datas = {
      { .texture_indices = {0, 0, 0, 0, 0, 0} },
      { .texture_indices = {2, 2, 2, 2, 1, 3} },
    },
  },
  m_chunk_generator_system(ChunkGeneratorSystem::create(seed)),
  m_chunk_mesher_system(ChunkMesherSystem::create()),
  m_chunk_renderer_system(ChunkRendererSystem::create()),
  m_light_system(LightSystem::create()),
  m_physics_system(PhysicsSystem::create()),
  m_player_movement_system(PlayerMovementSystem::create()),
  m_camera_follow_system(CameraFollowSystem::create()),
  m_debug_system(DebugSystem::create())
{}

void World::handle_event(SDL_Event event)
{
  m_player_movement_system->handle_event(*this, event);
}

void World::update(float dt)
{
  m_player_movement_system->update(*this, dt);
  m_light_system->update(*this);
  m_chunk_generator_system->update(*this);
  m_chunk_mesher_system->update(*this);
  m_physics_system->update(*this, dt);
  m_camera_follow_system->update(*this, dt);
}

void World::render()
{
  m_chunk_renderer_system->render(*this);
  m_debug_system->render(*this);
}

