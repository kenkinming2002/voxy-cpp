#include <graphics/texture.hpp>

#include <stb_image.h>

#include <experimental/scope>
#include <stdexcept>

#include <assert.h>

namespace graphics
{
  Texture::Texture(unsigned char *bytes, unsigned width, unsigned height, unsigned channels)
  {
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    switch(channels)
    {
      case 1: glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,  width, height, 0, GL_RED,  GL_UNSIGNED_BYTE, bytes); break;
      case 2: glTexImage2D(GL_TEXTURE_2D, 0, GL_RG,   width, height, 0, GL_RG,   GL_UNSIGNED_BYTE, bytes); break;
      case 3: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,  width, height, 0, GL_RGB,  GL_UNSIGNED_BYTE, bytes); break;
      case 4: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes); break;
    }
  }

  Texture::Texture(const std::string& filename)
  {
    stbi_set_flip_vertically_on_load(true);

    int width, height, channels;
    stbi_uc *bytes = stbi_load(filename.c_str(), &width, &height, &channels, 0);
    std::experimental::scope_exit bytes_exit([bytes]() { stbi_image_free(bytes); });

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    switch(channels)
    {
      case 3: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, bytes);   break;
      case 4: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes); break;
      default: throw std::runtime_error("Unexpected number of channel in image\n");
    }
    glGenerateMipmap(GL_TEXTURE_2D);
  }

  Texture::~Texture()
  {
    glDeleteTextures(1, &m_id);
  }
}

