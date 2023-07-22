#include <gl.hpp>
#include <glfw.hpp>
#include <camera.hpp>
#include <mesh.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdexcept>
#include <vector>
#include <fstream>
#include <sstream>
#include <utility>

#include <algorithm>
#include <math.h>

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

static const Vertex vertices[] = {
  {{-0.5f, -0.5f, -0.5f,}, { 0.0f, 0.0f,}, {1.0f, 1.0f, 1.0f}},
  {{0.5f, -0.5f, -0.5f, }, {1.0f, 0.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f,  0.5f, -0.5f, }, {1.0f, 1.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f,  0.5f, -0.5f, }, {1.0f, 1.0f, }, {1.0f, 1.0f, 1.0f}},
  {{-0.5f,  0.5f, -0.5f,}, { 0.0f, 1.0f,}, {1.0f, 1.0f, 1.0f}},
  {{-0.5f, -0.5f, -0.5f,}, { 0.0f, 0.0f,}, {1.0f, 1.0f, 1.0f}},

  {{-0.5f, -0.5f,  0.5f,}, { 0.0f, 0.0f,}, {1.0f, 1.0f, 1.0f}},
  {{0.5f, -0.5f,  0.5f, }, {1.0f, 0.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f,  0.5f,  0.5f, }, {1.0f, 1.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f,  0.5f,  0.5f, }, {1.0f, 1.0f, }, {1.0f, 1.0f, 1.0f}},
  {{-0.5f,  0.5f,  0.5f,}, { 0.0f, 1.0f,}, {1.0f, 1.0f, 1.0f}},
  {{-0.5f, -0.5f,  0.5f,}, { 0.0f, 0.0f,}, {1.0f, 1.0f, 1.0f}},

  {{-0.5f,  0.5f,  0.5f,}, { 1.0f, 0.0f,}, {1.0f, 1.0f, 1.0f}},
  {{-0.5f,  0.5f, -0.5f,}, { 1.0f, 1.0f,}, {1.0f, 1.0f, 1.0f}},
  {{-0.5f, -0.5f, -0.5f,}, { 0.0f, 1.0f,}, {1.0f, 1.0f, 1.0f}},
  {{-0.5f, -0.5f, -0.5f,}, { 0.0f, 1.0f,}, {1.0f, 1.0f, 1.0f}},
  {{-0.5f, -0.5f,  0.5f,}, { 0.0f, 0.0f,}, {1.0f, 1.0f, 1.0f}},
  {{-0.5f,  0.5f,  0.5f,}, { 1.0f, 0.0f,}, {1.0f, 1.0f, 1.0f}},

  {{0.5f,  0.5f,  0.5f, }, {1.0f, 0.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f,  0.5f, -0.5f, }, {1.0f, 1.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f, -0.5f, -0.5f, }, {0.0f, 1.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f, -0.5f, -0.5f, }, {0.0f, 1.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f, -0.5f,  0.5f, }, {0.0f, 0.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f,  0.5f,  0.5f, }, {1.0f, 0.0f, }, {1.0f, 1.0f, 1.0f}},

  {{-0.5f, -0.5f, -0.5f,}, { 0.0f, 1.0f,}, {1.0f, 1.0f, 1.0f}},
  {{0.5f, -0.5f, -0.5f, }, {1.0f, 1.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f, -0.5f,  0.5f, }, {1.0f, 0.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f, -0.5f,  0.5f, }, {1.0f, 0.0f, }, {1.0f, 1.0f, 1.0f}},
  {{-0.5f, -0.5f,  0.5f,}, { 0.0f, 0.0f,}, {1.0f, 1.0f, 1.0f}},
  {{-0.5f, -0.5f, -0.5f,}, { 0.0f, 1.0f,}, {1.0f, 1.0f, 1.0f}},

  {{-0.5f,  0.5f, -0.5f,}, { 0.0f, 1.0f,}, {1.0f, 1.0f, 1.0f}},
  {{0.5f,  0.5f, -0.5f, }, {1.0f, 1.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f,  0.5f,  0.5f, }, {1.0f, 0.0f, }, {1.0f, 1.0f, 1.0f}},
  {{0.5f,  0.5f,  0.5f, }, {1.0f, 0.0f, }, {1.0f, 1.0f, 1.0f}},
  {{-0.5f,  0.5f,  0.5f,}, { 0.0f, 0.0f,}, {1.0f, 1.0f, 1.0f}},
  {{-0.5f,  0.5f, -0.5f,}, { 0.0f, 1.0f }, {1.0f, 1.0f, 1.0f}},
};

static const uint16_t indices[] = {
  0, 1, 2, 3, 4, 5,
  6, 7, 8, 9, 10, 11,
  12, 13, 14, 15, 16, 17,
  18, 19, 20, 21, 22, 23,
  24, 25, 26, 27, 28, 29,
  30, 31, 32, 33, 34, 35,
};


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

  Mesh mesh(indices, vertices);
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

      mesh.draw();
    }

    glfwSwapBuffers(window);
  }
}
