project('voxy', ['c', 'cpp'], default_options : ['cpp_std=c++20'])

external_lib = static_library('external', [
    'external/src/glad.c',
    'external/src/stb_image.cpp',
  ],
  include_directories : 'external/include',
)

external_dep = declare_dependency(
  link_with : external_lib,
  include_directories : 'external/include',
)

sdl2_dep = dependency('sdl2')
freetype2_dep = dependency('freetype2')
glm_dep = dependency('glm')
fmt_dep = dependency('fmt')
spdlog_dep = dependency('spdlog')
openmp_dep = dependency('openmp')

voxy_exe = executable('voxy', [
    'src/camera.cpp',
    'src/text_renderer.cpp',
    'src/main.cpp',
    'src/application.cpp',
    'src/world.cpp',
    'src/graphics/mesh.cpp',
    'src/graphics/texture.cpp',
    'src/graphics/texture_array.cpp',
    'src/graphics/shader_program.cpp',
    'src/system/chunk_generator.cpp',
    'src/system/chunk_renderer.cpp',
    'src/system/light.cpp',
    'src/system/physics.cpp',
    'src/system/player_control.cpp',
    'src/system/player_ui.cpp',
    'src/system/camera_follow.cpp',
    'src/system/debug.cpp',
  ],
  include_directories : 'include',
  dependencies : [external_dep, sdl2_dep, freetype2_dep, glm_dep, fmt_dep, spdlog_dep, openmp_dep]
)
