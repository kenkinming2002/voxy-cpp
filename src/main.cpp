#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdexcept>
#include <vector>
#include <fstream>
#include <sstream>
#include <utility>

#include <algorithm>
#include <math.h>

namespace glfw
{
  struct Context
  {
    Context()
    {
      if(!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW");
    }

    ~Context()
    {
      glfwTerminate();
    }
  };

  struct Window
  {
    Window(const char *name, unsigned width, unsigned height)
    {
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
      window = glfwCreateWindow(width, height, name, nullptr, nullptr);
      if(!window)
        throw std::runtime_error("Failed to create window\n");
    }

    ~Window()
    {
      glfwDestroyWindow(window);
    }

    operator GLFWwindow *() { return window; }
    GLFWwindow *window;
  };
}

namespace gl
{
  struct VertexArray
  {
    VertexArray()  { glGenVertexArrays(1, &vertex_array); }
    ~VertexArray() { glDeleteVertexArrays(1, &vertex_array);  }

    VertexArray(const VertexArray& other) = delete;
    VertexArray& operator=(const VertexArray& other) = delete;

    VertexArray(VertexArray&& other) : vertex_array(0) { std::swap(vertex_array, other.vertex_array); };
    VertexArray& operator=(VertexArray&& other)        { std::swap(vertex_array, other.vertex_array); return *this; };

    operator GLuint() { return vertex_array; }
    GLuint vertex_array;
  };

  struct Buffer
  {
    Buffer()  { glGenBuffers(1, &buffer); }
    ~Buffer() { glDeleteBuffers(1, &buffer);  }

    Buffer(const Buffer& other) = delete;
    Buffer& operator=(const Buffer& other) = delete;

    Buffer(Buffer&& other) : buffer(0) { std::swap(buffer, other.buffer); };
    Buffer& operator=(Buffer&& other)  { std::swap(buffer, other.buffer); return *this; };

    operator GLuint() { return buffer; }
    GLuint buffer;
  };

  struct Shader
  {
    Shader(GLenum type) { shader = glCreateShader(type); }
    ~Shader()           { glDeleteShader(shader);  }

    Shader(const Shader& other) = delete;
    Shader& operator=(const Shader& other) = delete;

    Shader(Shader&& other) : shader(0) { std::swap(shader, other.shader); };
    Shader& operator=(Shader&& other)  { std::swap(shader, other.shader); return *this; };

    operator GLuint() { return shader; }
    GLuint shader;
  };

  struct Program
  {
    Program()  { program = glCreateProgram(); }
    ~Program() { glDeleteProgram(program);  }

    Program(const Program& other) = delete;
    Program& operator=(const Program& other) = delete;

    Program(Program&& other) : program(0) { std::swap(program, other.program); };
    Program& operator=(Program&& other)   { std::swap(program, other.program); return *this; };

    operator GLuint() { return program; }
    GLuint program;
  };

  struct Texture
  {
    Texture()  {
      glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    }
    ~Texture() { glDeleteTextures(1, &texture);  }

    Texture(const Texture& other) = delete;
    Texture& operator=(const Texture& other) = delete;

    Texture(Texture&& other) : texture(0) { std::swap(texture, other.texture); };
    Texture& operator=(Texture&& other)   { std::swap(texture, other.texture); return *this; };

    operator GLuint() { return texture; }
    GLuint texture;
  };

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

struct Camera
{
  glm::vec3 position = glm::vec3(0.0f, 0.0f,  3.0f);
  float yaw          = -90.0f;
  float pitch        = 0.0f;
  float fov          = 45.0f;

  glm::vec3 up() const
  {
    return glm::vec3(0.0f, 1.0f,  0.0f);
  }

  glm::vec3 forward() const
  {
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    return direction;
  }

  glm::vec3 right() const
  {
    return glm::normalize(glm::cross(forward(), up()));
  }

  glm::mat4 view() const
  {
    return glm::lookAt(position, position + forward(), up());
  }

  glm::mat4 projection() const
  {
    return glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);
  }

  static constexpr float MOVEMENT_SPEED = 2.5f;
  static constexpr float ROTATION_SPEED = 0.1f;

  void rotate(float x, float y)
  {
    yaw   += x * ROTATION_SPEED;
    pitch += y * ROTATION_SPEED;
    pitch = std::clamp(pitch, -89.0f, 89.0f);
  }

  void translate(float x, float y)
  {
    position += MOVEMENT_SPEED * (
        x * right() +
        y * forward()
    );
  }

  void zoom(float factor)
  {
    fov += factor;
    fov = std::clamp(fov, 1.0f, 45.0f);
  }
};

Camera camera;

bool first = true;
float last_xpos;
float last_ypos;


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  fprintf(stderr, "Callback\n");
  glViewport(0, 0, width, height);
}

void cursor_enter_callback(GLFWwindow* window, int entered)
{
  fprintf(stderr, "entered = %d\n", entered);
  first = entered;
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
  fprintf(stderr, "pos = %f, %f\n", xpos, ypos);
  if(first) {
    first = false;
    last_xpos = xpos;
    last_ypos = ypos;
    return;
  }

  float dx = xpos - last_xpos;
  float dy = ypos - last_ypos;

  last_xpos = xpos;
  last_ypos = ypos;

  camera.rotate(dx, -dy);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  camera.zoom(-yoffset);
}

int main()
{
  glfw::Context glfw_context;
  glfw::Window window("voxy", 800, 600);

  glfwMakeContextCurrent(window);
  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    throw std::runtime_error("Failed to load OpenGL functions with GLAD");

  gl::init_debug();

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorEnterCallback(window, cursor_enter_callback);
  glfwSetCursorPosCallback  (window, cursor_pos_callback);
  glfwSetScrollCallback     (window, scroll_callback);

  float vertices[] = {
      -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
       0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
       0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
       0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
      -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
       0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
       0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
       0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
      -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

      -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
      -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

       0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
       0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
       0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
       0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
       0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
       0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
       0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
       0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
       0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

      -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
       0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
       0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
       0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
      -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
  };

  glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f),
    glm::vec3( 2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3( 2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3( 1.3f, -2.0f, -2.5f),
    glm::vec3( 1.5f,  2.0f, -2.5f),
    glm::vec3( 1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
  };

  gl::VertexArray vao;
  gl::Buffer      vbo;

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0 * sizeof(float)));
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  gl::Texture texture0 = gl::load_texture("assets/container.jpg");
  gl::Texture texture1 = gl::load_texture("assets/awesomeface.png");

  gl::Program program = gl::compile_program("assets/shader.vert", "assets/shader.frag");

  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_DEPTH_TEST);

  float previous_time = glfwGetTime();
  float current_time;

  while(!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    current_time = glfwGetTime();
    float dt = current_time - previous_time;
    previous_time = current_time;

    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, GLFW_TRUE);

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.translate(0.0f,  dt);
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.translate(0.0f, -dt);
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.translate( dt, 0.0f);
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.translate(-dt, 0.0f);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);

    // Texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glUniform1i(glGetUniformLocation(program, "texture0"), 0);
    glUniform1i(glGetUniformLocation(program, "texture1"), 1);

    // Transform
    glBindVertexArray(vao);

    glm::mat4 view       = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    view       = camera.view();
    projection = camera.projection();

    for(size_t i=0; i<sizeof cubePositions / sizeof cubePositions[0]; ++i)
    {
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, cubePositions[i]);
      model = glm::rotate(model, glm::radians(i * 20.0f), glm::vec3(0.5f, 1.0f, 0.0f));
      if(i % 3 == 0)
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));

      glm::mat4 transform = projection * view * model;
      glUniformMatrix4fv(glGetUniformLocation(program, "transform"), 1, GL_FALSE, glm::value_ptr(transform));

      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glfwSwapBuffers(window);
  }
}
