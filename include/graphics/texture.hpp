#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <glad/glad.h>

#include <string>
#include <memory>

namespace graphics
{
  struct Texture
  {
  public:
    static std::unique_ptr<Texture> load_from(const std::string& filename);

  public:
    Texture(unsigned char *bytes, unsigned width, unsigned height, unsigned channels);
    ~Texture();

  public:
    GLuint id() const { return m_id; }

  private:
    GLuint m_id;
  };
}

#endif // TEXTURE_HPP
