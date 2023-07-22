project('voxy', ['c', 'cpp'], default_options : ['cpp_std=c++20'])

glfw_dep = dependency('glfw3')
glm_dep = dependency('glm')
voxy_exe = executable('voxy', [
    'src/camera.cpp',
    'src/gl.cpp',
    'src/glad.c',
    'src/glfw.cpp',
    'src/main.cpp',
    'src/mesh.cpp',
    'src/timer.cpp',
  ],
  include_directories : 'include',
  dependencies : [glfw_dep, glm_dep]
)
