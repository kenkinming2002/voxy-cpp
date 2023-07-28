#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct Transform
{
  glm::vec3 position;
  glm::quat rotation;

  static constexpr glm::vec3 UP = glm::vec3(0.0f, 0.0f, 1.0f); // Z-up coordinate system

  glm::vec3 local_right()   const { return glm::rotate(rotation, glm::vec3(1.0f, 0.0f, 0.0f)); }
  glm::vec3 local_forward() const { return glm::rotate(rotation, glm::vec3(0.0f, 1.0f, 0.0f)); }
  glm::vec3 local_up()      const { return glm::rotate(rotation, glm::vec3(0.0f, 0.0f, 1.0f)); }

  // See, this man is making up word
  glm::vec3 gocal_right()   const { glm::vec3 right   = local_right();   glm::vec3 gocal_right   = right   - glm::dot(right,   UP) * right;   if(glm::length2(gocal_right)   == 0.0f) return local_up(); return glm::normalize(gocal_right);   }
  glm::vec3 gocal_forward() const { glm::vec3 forward = local_forward(); glm::vec3 gocal_forward = forward - glm::dot(forward, UP) * forward; if(glm::length2(gocal_forward) == 0.0f) return local_up(); return glm::normalize(gocal_forward); }
  glm::vec3 gocal_up()      const { return UP;}

  glm::mat4 as_matrix() const
  {
    return glm::translate(glm::mat4(1.0f), position) * glm::toMat4(rotation);
  }

  Transform rotate(glm::quat rotation) const
  {
    Transform transform = *this;
    transform.rotation = rotation * transform.rotation;
    return transform;
  }

  Transform rotate(glm::vec3 rotation/* roll, pitch, yaw */) const
  {
    float roll  = rotation.x;
    float pitch = rotation.y;
    float yaw   = rotation.z;

    Transform transform = *this;
    transform.rotation = glm::angleAxis(glm::radians(yaw),   UP)                        * transform.rotation;
    transform.rotation = glm::angleAxis(glm::radians(pitch), transform.local_right())   * transform.rotation;
    transform.rotation = glm::angleAxis(glm::radians(roll),  transform.local_forward()) * transform.rotation;
    return transform;
  }

  glm::vec3 gocal_to_global(glm::vec3 translation) const
  {
    return translation.x * gocal_right() +
           translation.y * gocal_forward() +
           translation.z * gocal_up();
  }
};

#endif // TRANSFORM_HPP
