#ifndef SYSTEM_HPP
#define SYSTEM_HPP

struct Application;
struct WorldConfig;
struct WorldData;
class System
{
public:
  virtual void on_update(Application& application, const WorldConfig& config, WorldData& world, float dt)        {};
  virtual void on_render(Application& application, const WorldConfig& config, const WorldData& world)            {};
  virtual ~System() = default;
};

#endif // SYSTEM_HPP
