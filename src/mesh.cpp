#include <mesh.hpp>

Mesh::Mesh(std::span<const uint32_t> indices, VertexLayout vertex_layout, std::span<const std::byte> vertices)
{
  glBindVertexArray(vao);

  count = indices.size();
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size_bytes(), indices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  for(size_t i=0; i<vertex_layout.attributes.size(); ++i)
  {
    glEnableVertexAttribArray(i);
    glVertexAttribPointer(i, vertex_layout.attributes[i].count, GL_FLOAT, GL_FALSE, vertex_layout.stride, (void*)vertex_layout.attributes[i].offset);
  }
  glBufferData(GL_ARRAY_BUFFER, vertices.size_bytes(), vertices.data(), GL_STATIC_DRAW);
}

void Mesh::draw() const
{
  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void*)0);
}

