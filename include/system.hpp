#ifndef SYSTEM_HPP
#define SYSTEM_HPP

struct Application;
struct WorldConfig;
struct WorldData;
class System
{
public:
  virtual void on_start (Application& application, const WorldConfig& world_config, WorldData& world_data)           {};
  virtual void on_update(Application& application, const WorldConfig& world_config, WorldData& world_data, float dt) {};
  virtual void on_render(Application& application, const WorldConfig& world_config, const WorldData& world_data)     {};
  virtual ~System() = default;
};

#endif // SYSTEM_HPP
