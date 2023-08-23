#include <graphics/shader_program.hpp>

#include <fstream>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>

#include <experimental/scope>

namespace graphics
{
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
    GLuint vertex_shader   = compile_shader(GL_VERTEX_SHADER,   vertex_shader_path);   std::experimental::scope_exit vertex_shader_exit  ([vertex_shader]  (){ glDeleteShader(vertex_shader); });
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_path); std::experimental::scope_exit fragment_shader_exit([fragment_shader](){ glDeleteShader(fragment_shader); });

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

  void ShaderProgram::use() const
  {
    glUseProgram(m_id);
  }

  void ShaderProgram::set_uniform(const char* name, float value) { glUniform1f(glGetUniformLocation(m_id, name), value); }

  void ShaderProgram::set_uniform(const char* name, glm::vec2 value) { glUniform2fv(glGetUniformLocation(m_id, name), 1, glm::value_ptr(value)); }
  void ShaderProgram::set_uniform(const char* name, glm::vec3 value) { glUniform3fv(glGetUniformLocation(m_id, name), 1, glm::value_ptr(value)); }
  void ShaderProgram::set_uniform(const char* name, glm::vec4 value) { glUniform4fv(glGetUniformLocation(m_id, name), 1, glm::value_ptr(value)); }

  void ShaderProgram::set_uniform(const char* name, glm::mat2 value) { glUniformMatrix2fv(glGetUniformLocation(m_id, name), 1, GL_FALSE, glm::value_ptr(value)); }
  void ShaderProgram::set_uniform(const char* name, glm::mat3 value) { glUniformMatrix3fv(glGetUniformLocation(m_id, name), 1, GL_FALSE, glm::value_ptr(value)); }
  void ShaderProgram::set_uniform(const char* name, glm::mat4 value) { glUniformMatrix4fv(glGetUniformLocation(m_id, name), 1, GL_FALSE, glm::value_ptr(value)); }
}

