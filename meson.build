project('voxy', ['c', 'cpp'], default_options : ['cpp_std=c++20'])

sdl2_dep = dependency('sdl2')
freetype2_dep = dependency('freetype2')
glm_dep = dependency('glm')
fmt_dep = dependency('fmt')
spdlog_dep = dependency('spdlog')
voxy_exe = executable('voxy', [
    'src/camera.cpp',
    'src/gl.cpp',
    'src/glad.c',
    'src/mesh.cpp',
    'src/perlin.cpp',
    'src/sdl2.cpp',
    'src/text_renderer.cpp',
    'src/texture_array.cpp',
    'src/timer.cpp',
    'src/window.cpp',
    'src/main.cpp',
    'src/application.cpp',
    'src/system/chunk_generator.cpp',
    'src/system/chunk_mesher.cpp',
    'src/system/chunk_renderer.cpp',
    'src/system/light.cpp',
    'src/system/physics.cpp',
    'src/system/player_control.cpp',
    'src/system/player_ui.cpp',
    'src/system/camera_follow.cpp',
    'src/system/debug.cpp',
  ],
  include_directories : 'include',
  dependencies : [sdl2_dep, freetype2_dep, glm_dep, fmt_dep, spdlog_dep]
)
