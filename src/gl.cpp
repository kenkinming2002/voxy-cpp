#include <gl.hpp>

#include <fstream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace gl
{
  Shader::Shader(GLenum type) { handle = glCreateShader(type); }
  Shader::~Shader()           { glDeleteShader(handle);  }

  Program::Program()  { handle = glCreateProgram(); }
  Program::~Program() { glDeleteProgram(handle);  }

  Texture::Texture()  { glCreateTextures(GL_TEXTURE_2D, 1, &handle); }
  Texture::~Texture() { glDeleteTextures(1, &handle);  }

  gl::Shader compile_shader(GLenum type, const char* path)
  {
    gl::Shader shader(type);

    std::ifstream     ifs;
    std::stringstream ss;

    ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    ifs.open(path);
    ss << ifs.rdbuf();

    std::string source = ss.str();

    const GLchar *sources[] = { source.c_str() };
    glShaderSource(shader, 1, sources, nullptr);
    glCompileShader(shader);

    GLint success;
    GLchar info_log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
      glGetShaderInfoLog(shader, 512, nullptr, info_log);

      std::string message;
      message.append("Failed to compile OpenGL shader:\n");
      message.append(info_log);
      throw std::runtime_error(std::move(message));
    }

    return shader;
  }

  gl::Program compile_program(const char* vertex_shader_path, const char *fragment_shader_path)
  {
    gl::Program program;

    gl::Shader vertex_shader   = compile_shader(GL_VERTEX_SHADER, vertex_shader_path);
    gl::Shader fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_path);

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint success;
    GLchar info_log[512] = {};
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success) {
      glGetProgramInfoLog(program, 512, nullptr, info_log);

      std::string message;
      message.append("Failed to link OpenGL program:");
      message.append(info_log);
      throw std::runtime_error(std::move(message));
    }

    return program;
  }

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

