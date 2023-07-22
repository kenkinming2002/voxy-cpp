#ifndef MESH_HPP
#define MESH_HPP

#include <glm/glm.hpp>
#include <gl.hpp>

#include <span>

struct Vertex
{
  glm::vec3 pos;
  glm::vec2 uv;
  glm::vec3 color;
};

struct Mesh
{
  gl::VertexArray vao;
  gl::Buffer ebo;
  gl::Buffer vbo;

  size_t count;

  Mesh(std::span<const uint16_t> indices, std::span<const Vertex> vertices);
  void draw();
};


#endif // MESH_HPP
