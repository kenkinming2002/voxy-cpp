#ifndef MESH_HPP
#define MESH_HPP

#include <glm/glm.hpp>
#include <glad/glad.h>

#include <vector>
#include <memory>
#include <span>

enum class IndexType {
  UNSIGNED_BYTE,
  UNSIGNED_SHORT,
  UNSIGNED_INT,
};

enum class AttributeType {
  FLOAT1,
  FLOAT2,
  FLOAT3,
  FLOAT4,

  UNSIGNED_INT1,
  UNSIGNED_INT2,
  UNSIGNED_INT3,
  UNSIGNED_INT4,
};

struct Attribute
{
  AttributeType type;
  size_t        offset;
};

struct MeshLayout
{
  IndexType              index_type;

  size_t                 stride;
  std::vector<Attribute> attributes;
};

struct Mesh
{
public:
  Mesh(MeshLayout layout, std::vector<std::byte> indices, std::vector<std::byte> vertices);
  ~Mesh();

public:
  void draw() const;

private:
  MeshLayout m_layout;

  mutable std::vector<std::byte> m_indices;
  mutable std::vector<std::byte> m_vertices;

  mutable bool m_generated;

  mutable GLuint m_vao;
  mutable GLuint m_ebo;
  mutable GLuint m_vbo;

  mutable GLsizei m_element_count;
};

template<typename T>
inline static std::vector<std::byte> as_bytes(const std::vector<T>& data) requires(std::is_pod_v<T>)
{
  auto bytes = std::as_bytes(std::span(data));
  return std::vector(bytes.begin(), bytes.end());
}

#endif // MESH_HPP
