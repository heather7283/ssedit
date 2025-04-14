project('ssedit', 'cpp',
        version: '0.0.1',
        default_options: ['warning_level=3', 'cpp_std=c++23'])

add_project_arguments('-DIMGUI_DEFINE_MATH_OPERATORS', language: 'cpp')

gl_dep = dependency('gl')
glew_dep = dependency('glew')
glfw_dep = dependency('glfw3')

include_dirs = [
  'thirdparty',
  'thirdparty/imgui',
  'thirdparty/imgui/backends',
]

imgui_sources = [
  'thirdparty/imgui/imgui.cpp',
  'thirdparty/imgui/imgui_draw.cpp',
  'thirdparty/imgui/imgui_tables.cpp',
  'thirdparty/imgui/imgui_widgets.cpp',
  'thirdparty/imgui/backends/imgui_impl_glfw.cpp',
  'thirdparty/imgui/backends/imgui_impl_opengl3.cpp'
]

imgui_lib = static_library('imgui', imgui_sources,
                           include_directories: include_dirs)

ssedit_sources = [
  'src/ssedit.cpp',
  'src/log.cpp',
  'src/stb_image_impl.cpp',
  'src/stb_image_write_impl.cpp',
]

executable('ssedit', ssedit_sources,
           include_directories: include_dirs,
           link_with: [imgui_lib],
           dependencies: [glfw_dep, gl_dep, glew_dep],
           install: true)

