#include <graphics/mesh.hpp>

#include <tiny_obj_loader.h>

#include <spdlog/spdlog.h>

#include <iostream>

namespace graphics
{
  std::unique_ptr<Mesh> Mesh::load_from(const std::string& filename)
  {
    struct Vertex
    {
      glm::vec3 position;
      glm::vec3 normal;
      glm::vec2 texture_coords;
    };

    tinyobj::ObjReader reader;
    reader.ParseFromFile(filename);

    const auto& attrib = reader.GetAttrib();

    std::vector<Vertex>   vertices;
    std::vector<unsigned> indices;

    const auto& shapes = reader.GetShapes();
    for(const auto& shape : shapes)
      for(const auto& index : shape.mesh.indices)
      {
        vertices.push_back(Vertex{
          .position = glm::vec3(
              attrib.vertices[index.vertex_index*3+0],
              attrib.vertices[index.vertex_index*3+1],
              attrib.vertices[index.vertex_index*3+2]
          ),
          .normal = glm::vec3(
              attrib.normals[index.normal_index*3+0],
              attrib.normals[index.normal_index*3+1],
              attrib.normals[index.normal_index*3+2]
          ),
          .texture_coords = glm::vec2(
              attrib.texcoords[index.texcoord_index*2+0],
              attrib.texcoords[index.texcoord_index*2+1]
          ),
        });
        indices.push_back(indices.size());
      }

    const Attribute attributes[] = {
      { .type = graphics::AttributeType::FLOAT3, .offset = offsetof(Vertex, position),       },
      { .type = graphics::AttributeType::FLOAT3, .offset = offsetof(Vertex, normal),         },
      { .type = graphics::AttributeType::FLOAT2, .offset = offsetof(Vertex, texture_coords), },
    };

    std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>(
        IndexType::UNSIGNED_INT,
        PrimitiveType::TRIANGLES,
        sizeof(Vertex),
        attributes
    );
    mesh->write(std::as_bytes(std::span(indices)), std::as_bytes(std::span(vertices)), Usage::STATIC);
    return mesh;
  }

  Mesh::Mesh(IndexType index_type, PrimitiveType primitive_type, size_t stride, std::span<const Attribute> attributes) :
    m_index_type(index_type),
    m_primitive_type(primitive_type),
    m_element_count(0)
  {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_ebo);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    for(std::size_t i=0; i<attributes.size(); ++i)
    {
      glEnableVertexAttribArray(i);
      switch(attributes[i].type)
      {
        case AttributeType::FLOAT1: glVertexAttribPointer(i, 1, GL_FLOAT, GL_FALSE, stride, (void*)attributes[i].offset); break;
        case AttributeType::FLOAT2: glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, stride, (void*)attributes[i].offset); break;
        case AttributeType::FLOAT3: glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, stride, (void*)attributes[i].offset); break;
        case AttributeType::FLOAT4: glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, stride, (void*)attributes[i].offset); break;

        case AttributeType::UNSIGNED_INT1: glVertexAttribIPointer(i, 1, GL_UNSIGNED_INT, stride, (void*)attributes[i].offset); break;
        case AttributeType::UNSIGNED_INT2: glVertexAttribIPointer(i, 2, GL_UNSIGNED_INT, stride, (void*)attributes[i].offset); break;
        case AttributeType::UNSIGNED_INT3: glVertexAttribIPointer(i, 3, GL_UNSIGNED_INT, stride, (void*)attributes[i].offset); break;
        case AttributeType::UNSIGNED_INT4: glVertexAttribIPointer(i, 4, GL_UNSIGNED_INT, stride, (void*)attributes[i].offset); break;
      }
    }
  }

  Mesh::~Mesh()
  {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_ebo);
    glDeleteBuffers(1, &m_vbo);
  }

  void Mesh::write(std::span<const std::byte> indices, std::span<const std::byte> vertices, Usage usage)
  {
    GLenum _usage;
    switch(usage)
    {
    case Usage::STATIC:  _usage = GL_STATIC_DRAW; break;
    case Usage::DYNAMIC: _usage = GL_DYNAMIC_DRAW; break;
    case Usage::STREAM:  _usage = GL_STREAM_DRAW; break;
    }

    glBindVertexArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size(), indices.data(), _usage);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size(), vertices.data(), _usage);

    switch(m_index_type)
    {
    case IndexType::UNSIGNED_BYTE:  m_element_count = indices.size() / 1; break;
    case IndexType::UNSIGNED_SHORT: m_element_count = indices.size() / 2; break;
    case IndexType::UNSIGNED_INT:   m_element_count = indices.size() / 4; break;
    }
  }

  void Mesh::draw() const
  {
    GLenum mode, type;
    switch(m_index_type)
    {
    case IndexType::UNSIGNED_BYTE:  type = GL_UNSIGNED_BYTE;  break;
    case IndexType::UNSIGNED_SHORT: type = GL_UNSIGNED_SHORT; break;
    case IndexType::UNSIGNED_INT:   type = GL_UNSIGNED_INT;   break;
    }
    switch(m_primitive_type)
    {
    case PrimitiveType::LINES:     mode = GL_LINES;     break;
    case PrimitiveType::TRIANGLES: mode = GL_TRIANGLES; break;
    }

    glBindVertexArray(m_vao);
    glDrawElements(mode, m_element_count, type, (void*)0);
  }
}

