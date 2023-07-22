project('voxy', ['c', 'cpp'], default_options : ['cpp_std=c++20'])

glfw_dep = dependency('glfw3')
glm_dep = dependency('glm')
voxy_exe = executable('voxy', [
    'src/main.cpp',
    'src/glfw.cpp',
    'src/gl.cpp',
    'src/camera.cpp',
    'src/mesh.cpp',
    'src/glad.c',
  ],
  include_directories : 'include',
  dependencies : [glfw_dep, glm_dep]
)
