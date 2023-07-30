project('voxy', ['c', 'cpp'], default_options : ['cpp_std=c++20'])

sdl2_dep = dependency('sdl2')
freetype2_dep = dependency('freetype2')
glm_dep = dependency('glm')
fmt_dep = dependency('fmt')
spdlog_dep = dependency('spdlog')
voxy_exe = executable('voxy', [
    'src/gl.cpp',
    'src/sdl2.cpp',
    'src/glad.c',
    'src/main.cpp',
    'src/mesh.cpp',
    'src/texture_array.cpp',
    'src/text_renderer.cpp',
    'src/perlin.cpp',
    'src/chunk.cpp',
    'src/chunk/generator.cpp',
    'src/dimension.cpp',
    'src/world.cpp',
    'src/timer.cpp',
    'src/camera.cpp',
  ],
  include_directories : 'include',
  dependencies : [sdl2_dep, freetype2_dep, glm_dep, fmt_dep, spdlog_dep]
)
