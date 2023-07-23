#ifndef MESH_HPP
#define MESH_HPP

#include <glm/glm.hpp>
#include <gl.hpp>

#include <vector>
#include <span>

struct VertexAttribute
{
  size_t offset;
  size_t count;
};

struct VertexLayout
{
  size_t stride;
  std::vector<VertexAttribute> attributes;
};

struct Mesh
{
  gl::VertexArray vao;
  gl::Buffer ebo;
  gl::Buffer vbo;

  size_t count;

  Mesh(std::span<const uint32_t> indices, VertexLayout vertex_layout, std::span<const std::byte> vertices);

  void draw() const;
};


#endif // MESH_HPP
