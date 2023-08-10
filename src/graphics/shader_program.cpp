#include <graphics/shader_program.hpp>

#include <fstream>
#include <sstream>

#include <experimental/scope>

static GLuint compile_shader(GLenum type, const char *path)
{
  std::ifstream     ifs;
  std::stringstream ss;

  ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  ifs.open(path);
  ss << ifs.rdbuf();

  std::string source = ss.str();
  const GLchar *sources[] = { source.c_str() };

  GLuint shader = glCreateShader(type);
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

static GLuint link_program(const char* vertex_shader_path, const char *fragment_shader_path)
{
  GLuint vertex_shader   = compile_shader(GL_VERTEX_SHADER,   vertex_shader_path);   //std::experimental::scope_exit vertex_shader_exit  ([vertex_shader]  (){ glDeleteShader(vertex_shader); });
  GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_path); //std::experimental::scope_exit fragment_shader_exit([fragment_shader](){ glDeleteShader(fragment_shader); });

  GLuint program = glCreateProgram();
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

ShaderProgram::ShaderProgram(const char *vertex_shader_path, const char *fragment_shader_path) : m_id(link_program(vertex_shader_path, fragment_shader_path)) { }
ShaderProgram::~ShaderProgram() { glDeleteProgram(m_id); }
