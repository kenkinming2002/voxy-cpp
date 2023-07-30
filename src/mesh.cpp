#include <mesh.hpp>

#include <spdlog/spdlog.h>

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

void Mesh::draw() const
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

