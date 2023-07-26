project('voxy', ['c', 'cpp'], default_options : ['cpp_std=c++20'])

sdl2_dep = dependency('sdl2')
glm_dep = dependency('glm')
voxy_exe = executable('voxy', [
    'src/gl.cpp',
    'src/sdl2.cpp',
    'src/glad.c',
    'src/main.cpp',
    'src/mesh.cpp',
    'src/perlin.cpp',
    'src/world.cpp',
    'src/timer.cpp',
    'src/camera.cpp',
  ],
  include_directories : 'include',
  dependencies : [sdl2_dep, glm_dep]
)
