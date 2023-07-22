project('voxy', ['c', 'cpp'])

glfw_dep = dependency('glfw3')
glm_dep = dependency('glm')
voxy_exe = executable('voxy', [
    'src/main.cpp',
    'src/glfw.cpp',
    'src/gl.cpp',
    'src/glad.c'
  ],
  include_directories : 'include',
  dependencies : [glfw_dep, glm_dep]
)
