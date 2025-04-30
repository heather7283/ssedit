project('ssedit', 'cpp',
        version: '0.0.1',
        license: 'GPL-3.0-or-later',
        meson_version: '>=1.1',
        default_options: ['warning_level=3', 'cpp_std=c++23'])

add_project_arguments('-Wno-unused-parameter', language: 'cpp')
add_project_arguments('-DSSEDIT_VERSION="@0@"'.format(meson.project_version()), language: 'cpp')

gl_dep = dependency('gl')
glew_dep = dependency('glew')
glfw_dep = dependency('glfw3')

spng_lib = dependency('spng', required: get_option('png'))
if spng_lib.found()
  add_project_arguments([
    '-DSSEDIT_HAVE_LIBSPNG',
    '-DSSEDIT_LIBSPNG_VERSION="@0@"'.format(spng_lib.version()),
  ], language: 'cpp')
endif

turbojpeg_lib = dependency('libturbojpeg', required: get_option('jpeg'))
if turbojpeg_lib.found()
  add_project_arguments([
    '-DSSEDIT_HAVE_LIBTURBOJPEG',
    '-DSSEDIT_LIBTURBOJPEG_VERSION="@0@"'.format(turbojpeg_lib.version()),
  ], language: 'cpp')
endif

format_libs = [ spng_lib, turbojpeg_lib ]
any_format_enabled = false
foreach format_lib: format_libs
  if format_lib.found()
    any_format_enabled = true
    break
  endif
endforeach
if not any_format_enabled
  error('You must enable support for at least one image format')
endif

include_dirs = [
  'thirdparty',
  'thirdparty/imgui',
  'thirdparty/imgui/backends',
  'src'
]

imgui_sources = [
  'thirdparty/imgui/imgui.cpp',
  'thirdparty/imgui/imgui_draw.cpp',
  'thirdparty/imgui/imgui_tables.cpp',
  'thirdparty/imgui/imgui_widgets.cpp',
  'thirdparty/imgui/backends/imgui_impl_glfw.cpp',
  'thirdparty/imgui/backends/imgui_impl_opengl3.cpp'
]

add_project_arguments('-DIMGUI_DEFINE_MATH_OPERATORS', language: 'cpp')
imgui_lib = static_library('imgui', imgui_sources, include_directories: include_dirs)

ssedit_sources = [
  'src/ssedit.cpp',
  'src/shapes.cpp',
  'src/decode.cpp',
  'src/encode.cpp',
  'src/utils.cpp',
  'src/formats.cpp',
  'src/clipboard.cpp',
  'src/log.cpp',
  'src/backends/jpeg.cpp',
  'src/backends/png.cpp',
]

executable('ssedit', ssedit_sources,
           include_directories: include_dirs,
           link_with: [imgui_lib],
           dependencies: [glfw_dep, gl_dep, glew_dep, spng_lib, turbojpeg_lib],
           install: true)

summary({'PNG': spng_lib.found(),
         'JPEG': turbojpeg_lib.found()}, section: 'Supported image formats')

