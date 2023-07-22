project('voxy', ['c', 'cpp'], default_options : ['cpp_std=c++20'])

sdl2_dep = dependency('sdl2')
glm_dep = dependency('glm')
voxy_exe = executable('voxy', [
    'src/camera.cpp',
    'src/gl.cpp',
    'src/glad.c',
    'src/sdl2.cpp',
    'src/main.cpp',
    'src/mesh.cpp',
    'src/world.cpp',
    'src/timer.cpp',
  ],
  include_directories : 'include',
  dependencies : [sdl2_dep, glm_dep]
)
