#ifndef SYSTEM_CAMERA_FOLLOW_HPP
#define SYSTEM_CAMERA_FOLLOW_HPP

#include <memory>
#include <stddef.h>

struct World;
class CameraFollowSystem
{
public:
  static std::unique_ptr<CameraFollowSystem> create();

public:
  virtual void update(World& world, float dt) = 0;
  virtual ~CameraFollowSystem() = default;
};

#endif // SYSTEM_CAMERA_FOLLOW_HPP

