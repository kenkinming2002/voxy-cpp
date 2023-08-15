#ifndef SYSTEM_HPP
#define SYSTEM_HPP

struct Application;
struct WorldConfig;
struct World;
struct Camera;
class System
{
public:
  virtual void on_start (Application& application, const WorldConfig& world_config,       World& world_data)                       {};
  virtual void on_update(Application& application, const WorldConfig& world_config,       World& world_data, float dt)             {};
  virtual void on_render(Application& application, const WorldConfig& world_config, const World& world_data, const Camera& camera) {};
  virtual ~System() = default;
};

#endif // SYSTEM_HPP
