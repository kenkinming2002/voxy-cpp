#ifndef MESH_HPP
#define MESH_HPP

#include <glm/glm.hpp>
#include <glad/glad.h>

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
  GLuint vao = 0;
  GLuint ebo = 0;
  GLuint vbo = 0;

  size_t count;

  Mesh(std::span<const uint32_t> indices, VertexLayout vertex_layout, std::span<const std::byte> vertices);
  ~Mesh();

  void draw() const;
};


#endif // MESH_HPP
