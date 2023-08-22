#include <graphics/mesh.hpp>

#include <tiny_obj_loader.h>

#include <spdlog/spdlog.h>

#include <iostream>

namespace graphics
{
  Mesh::Mesh(const std::string& filename)
  {
    struct Vertex
    {
      glm::vec3 position;
      glm::vec3 normal;
      glm::vec2 texture_coords;
    };

    m_layout = {
      .index_type = graphics::IndexType::UNSIGNED_INT,
      .stride = sizeof(Vertex),
      .attributes = {
        { .type = graphics::AttributeType::FLOAT3,        .offset = offsetof(Vertex, position),       },
        { .type = graphics::AttributeType::FLOAT3,        .offset = offsetof(Vertex, normal),         },
        { .type = graphics::AttributeType::FLOAT2,        .offset = offsetof(Vertex, texture_coords), },
      },
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

    m_vertices = as_bytes(vertices);
    m_indices  = as_bytes(indices);

    m_generated = false;
    m_vao = 0;
    m_ebo = 0;
    m_vbo = 0;
  }

  Mesh::Mesh(MeshLayout layout, std::vector<std::byte> indices, std::vector<std::byte> vertices) :
    m_layout(std::move(layout)),
    m_indices(std::move(indices)),
    m_vertices(std::move(vertices)),
    m_generated(false),
    m_vao(0),
    m_ebo(0),
    m_vbo(0)
  {}

  Mesh::~Mesh()
  {
    if(m_generated)
    {
      glDeleteVertexArrays(1, &m_vao);
      glDeleteBuffers(1, &m_ebo);
      glDeleteBuffers(1, &m_vbo);
    }
  }

  void Mesh::draw_triangles() const
  {
    if(!m_generated)
    {
      spdlog::info("Lazily generating mesh with index buffer size = {}, vertex buffer size = {}",
          m_indices.size(),
          m_vertices.size());

      glGenVertexArrays(1, &m_vao);
      glGenBuffers(1, &m_ebo);
      glGenBuffers(1, &m_vbo);

      glBindVertexArray(m_vao);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size(), m_indices.data(), GL_STATIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
      glBufferData(GL_ARRAY_BUFFER, m_vertices.size(), m_vertices.data(), GL_STATIC_DRAW);

      switch(m_layout.index_type)
      {
      case IndexType::UNSIGNED_BYTE:  m_element_count = m_indices.size() / 1; break;
      case IndexType::UNSIGNED_SHORT: m_element_count = m_indices.size() / 2; break;
      case IndexType::UNSIGNED_INT:   m_element_count = m_indices.size() / 4; break;
      }

      for(std::size_t i=0; i<m_layout.attributes.size(); ++i)
      {
        glEnableVertexAttribArray(i);
        switch(m_layout.attributes[i].type)
        {
        case AttributeType::FLOAT1: glVertexAttribPointer(i, 1, GL_FLOAT, GL_FALSE, m_layout.stride, (void*)m_layout.attributes[i].offset); break;
        case AttributeType::FLOAT2: glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, m_layout.stride, (void*)m_layout.attributes[i].offset); break;
        case AttributeType::FLOAT3: glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, m_layout.stride, (void*)m_layout.attributes[i].offset); break;
        case AttributeType::FLOAT4: glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, m_layout.stride, (void*)m_layout.attributes[i].offset); break;

        case AttributeType::UNSIGNED_INT1: glVertexAttribIPointer(i, 1, GL_UNSIGNED_INT, m_layout.stride, (void*)m_layout.attributes[i].offset); break;
        case AttributeType::UNSIGNED_INT2: glVertexAttribIPointer(i, 2, GL_UNSIGNED_INT, m_layout.stride, (void*)m_layout.attributes[i].offset); break;
        case AttributeType::UNSIGNED_INT3: glVertexAttribIPointer(i, 3, GL_UNSIGNED_INT, m_layout.stride, (void*)m_layout.attributes[i].offset); break;
        case AttributeType::UNSIGNED_INT4: glVertexAttribIPointer(i, 4, GL_UNSIGNED_INT, m_layout.stride, (void*)m_layout.attributes[i].offset); break;
        }
      }

      m_indices.clear();
      m_vertices.clear();

      m_generated = true;
    }

    glBindVertexArray(m_vao);
    switch(m_layout.index_type)
    {
    case IndexType::UNSIGNED_BYTE:  glDrawElements(GL_TRIANGLES, m_element_count, GL_UNSIGNED_BYTE,  (void*)0); break;
    case IndexType::UNSIGNED_SHORT: glDrawElements(GL_TRIANGLES, m_element_count, GL_UNSIGNED_SHORT, (void*)0); break;
    case IndexType::UNSIGNED_INT:   glDrawElements(GL_TRIANGLES, m_element_count, GL_UNSIGNED_INT,   (void*)0); break;
    }
  }

  void Mesh::draw_lines() const
  {
    if(!m_generated)
    {
      spdlog::info("Lazily generating mesh with index buffer size = {}, vertex buffer size = {}",
          m_indices.size(),
          m_vertices.size());

      glGenVertexArrays(1, &m_vao);
      glGenBuffers(1, &m_ebo);
      glGenBuffers(1, &m_vbo);

      glBindVertexArray(m_vao);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size(), m_indices.data(), GL_STATIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
      glBufferData(GL_ARRAY_BUFFER, m_vertices.size(), m_vertices.data(), GL_STATIC_DRAW);

      switch(m_layout.index_type)
      {
      case IndexType::UNSIGNED_BYTE:  m_element_count = m_indices.size() / 1; break;
      case IndexType::UNSIGNED_SHORT: m_element_count = m_indices.size() / 2; break;
      case IndexType::UNSIGNED_INT:   m_element_count = m_indices.size() / 4; break;
      }

      for(std::size_t i=0; i<m_layout.attributes.size(); ++i)
      {
        glEnableVertexAttribArray(i);
        switch(m_layout.attributes[i].type)
        {
        case AttributeType::FLOAT1: glVertexAttribPointer(i, 1, GL_FLOAT, GL_FALSE, m_layout.stride, (void*)m_layout.attributes[i].offset); break;
        case AttributeType::FLOAT2: glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, m_layout.stride, (void*)m_layout.attributes[i].offset); break;
        case AttributeType::FLOAT3: glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, m_layout.stride, (void*)m_layout.attributes[i].offset); break;
        case AttributeType::FLOAT4: glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, m_layout.stride, (void*)m_layout.attributes[i].offset); break;

        case AttributeType::UNSIGNED_INT1: glVertexAttribIPointer(i, 1, GL_UNSIGNED_INT, m_layout.stride, (void*)m_layout.attributes[i].offset); break;
        case AttributeType::UNSIGNED_INT2: glVertexAttribIPointer(i, 2, GL_UNSIGNED_INT, m_layout.stride, (void*)m_layout.attributes[i].offset); break;
        case AttributeType::UNSIGNED_INT3: glVertexAttribIPointer(i, 3, GL_UNSIGNED_INT, m_layout.stride, (void*)m_layout.attributes[i].offset); break;
        case AttributeType::UNSIGNED_INT4: glVertexAttribIPointer(i, 4, GL_UNSIGNED_INT, m_layout.stride, (void*)m_layout.attributes[i].offset); break;
        }
      }

      m_indices.clear();
      m_vertices.clear();

      m_generated = true;
    }

    glBindVertexArray(m_vao);
    switch(m_layout.index_type)
    {
    case IndexType::UNSIGNED_BYTE:  glDrawElements(GL_LINES, m_element_count, GL_UNSIGNED_BYTE,  (void*)0); break;
    case IndexType::UNSIGNED_SHORT: glDrawElements(GL_LINES, m_element_count, GL_UNSIGNED_SHORT, (void*)0); break;
    case IndexType::UNSIGNED_INT:   glDrawElements(GL_LINES, m_element_count, GL_UNSIGNED_INT,   (void*)0); break;
    }
  }
}

