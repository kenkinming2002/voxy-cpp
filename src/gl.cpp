#include <gl.hpp>

#include <fstream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace gl
{
  Texture::Texture()  { glCreateTextures(GL_TEXTURE_2D, 1, &handle); }
  Texture::~Texture() { glDeleteTextures(1, &handle);  }

  gl::Texture load_texture(const char *path)
  {
    gl::Texture texture;

    int width, height, channels;

    stbi_set_flip_vertically_on_load(true);
    stbi_uc *bytes = stbi_load(path, &width, &height, &channels, 0);

    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if(channels == 3) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, bytes);
    } else if(channels == 4) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
    } else {
      stbi_image_free(bytes);
      throw std::runtime_error("Unexpected number of channel in image\n");
    }
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(bytes);
    return texture;
  }

  void GLAPIENTRY message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
  {
    fprintf(stderr, "OpenGL Error: type = %u: %s\n", type, message);
  }

  void init_debug()
  {
    glDebugMessageCallback(message_callback, 0);
  }
}

