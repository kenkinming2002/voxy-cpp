#ifndef MESH_HPP
#define MESH_HPP

#include <glm/glm.hpp>
#include <glad/glad.h>

#include <string>
#include <vector>
#include <span>
#include <memory>

#include <stddef.h>

namespace graphics
{
  enum class IndexType {
    UNSIGNED_BYTE,
    UNSIGNED_SHORT,
    UNSIGNED_INT,
  };

  enum class PrimitiveType {
    LINES,
    TRIANGLES,
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

  enum class Usage {
    STATIC,
    DYNAMIC,
    STREAM,
  };

  struct Mesh
  {
  public:
    static std::unique_ptr<Mesh> load_from(const std::string& filename);

  public:
    Mesh(IndexType index_type, PrimitiveType primitive_type, size_t stride, std::span<const Attribute> attributes);
    ~Mesh();

  public:
    void write(std::span<const std::byte> indices, std::span<const std::byte> vertices, Usage usage);
    void draw() const;

  private:
    IndexType     m_index_type;
    PrimitiveType m_primitive_type;
    size_t        m_element_count;

    GLuint m_vao;
    GLuint m_ebo;
    GLuint m_vbo;
  };
}

#endif // MESH_HPP
