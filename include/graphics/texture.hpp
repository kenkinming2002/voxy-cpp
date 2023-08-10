#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <glad/glad.h>

#include <string>

struct Texture
{
public:
  Texture(unsigned char *bytes, unsigned width, unsigned height, unsigned channels);
  Texture(const std::string& filename);
  ~Texture();

public:
  GLuint id() const { return m_id; }

private:
  GLuint m_id;
};

#endif // TEXTURE_HPP
