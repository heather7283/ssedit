project('ssedit', ['c', 'cpp'],
        version: '0.0.1',
        license: 'GPL-3.0-or-later',
        meson_version: '>=1.1',
        default_options: ['warning_level=3', 'cpp_std=c++23'])

add_project_arguments('-Wno-unused-parameter', language: 'cpp')
add_project_arguments('-DSSEDIT_VERSION="@0@"'.format(meson.project_version()), language: 'cpp')

gl_dep = dependency('gl')
glew_dep = dependency('glew')
glfw_dep = dependency('glfw3')

any_format_enabled = false

spng_lib = dependency('spng', required: get_option('png'))
if spng_lib.found()
  add_project_arguments([
    '-DSSEDIT_HAVE_LIBSPNG',
    '-DSSEDIT_LIBSPNG_VERSION="@0@"'.format(spng_lib.version()),
  ], language: 'cpp')
  any_format_enabled = true
endif

turbojpeg_lib = dependency('libturbojpeg', required: get_option('jpeg'))
if turbojpeg_lib.found()
  add_project_arguments([
    '-DSSEDIT_HAVE_LIBTURBOJPEG',
    '-DSSEDIT_LIBTURBOJPEG_VERSION="@0@"'.format(turbojpeg_lib.version()),
  ], language: 'cpp')
  any_format_enabled = true
endif

jxl_lib = dependency('libjxl', required: get_option('jpegxl'))
jxl_threads_lib = dependency('libjxl_threads', required: get_option('jpegxl'))
if jxl_lib.found() and jxl_threads_lib.found()
  add_project_arguments([
    '-DSSEDIT_HAVE_LIBJXL',
    '-DSSEDIT_LIBJXL_VERSION="@0@"'.format(jxl_lib.version()),
  ], language: 'cpp')
  any_format_enabled = true
endif

image_format_libs = [spng_lib, turbojpeg_lib, jxl_lib, jxl_threads_lib]

if not any_format_enabled
  error('You must enable support for at least one image format')
endif

wl_copy_prog = find_program('wl-copy', required: false)
if not wl_copy_prog.found()
  warning('wl-copy is not found but it is required for builtin clipboard copy')
endif

include_dirs = [
  'thirdparty',
  'thirdparty/imgui',
  'thirdparty/imgui/backends',
  'thirdparty/inih',
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

inih_lib = static_library('inih', 'thirdparty/inih/ini.c', include_directories: include_dirs)

fontforge = find_program('fontforge')
icons_ttf = custom_target('icons.ttf',
                          input: 'thirdparty/fontawesome/fa-solid-900.ttf',
                          output: 'icons.ttf',
                          depend_files: ['scripts/strip_font.py', 'scripts/glyphs.txt'],
                          command: [fontforge, '-script', '@SOURCE_ROOT@/scripts/strip_font.py',
                                    '@INPUT@', '@OUTPUT@', '@SOURCE_ROOT@/scripts/glyphs.txt'])

objcopy = find_program('objcopy')
icons_obj = custom_target('icons.o',
                          input: icons_ttf,
                          output: 'icons.o',
                          command: [objcopy, '-I', 'binary', '-O', 'elf64-x86-64', '-B', 'i386',
                                    '--rename-section', '.data=.rodata',
                                    '--add-section', '.note.GNU-stack=/dev/null',
                                    '@INPUT@', '@OUTPUT@'])

ssedit_sources = [
  'src/ssedit.cpp',
  'src/shapes.cpp',
  'src/decode.cpp',
  'src/encode.cpp',
  'src/utils.cpp',
  'src/formats.cpp',
  'src/clipboard.cpp',
  'src/image.cpp',
  'src/icons.cpp',
  'src/config.cpp',
  'src/log.cpp',
  'src/backends/jpeg.cpp',
  'src/backends/png.cpp',
  'src/backends/jxl.cpp',
]

executable('ssedit', ssedit_sources + icons_obj,
           include_directories: include_dirs,
           link_with: [imgui_lib, inih_lib],
           dependencies: [glfw_dep, gl_dep, glew_dep] + image_format_libs,
           install: true)

summary({'PNG': spng_lib.found(),
         'JPEG': turbojpeg_lib.found(),
         'JXL': jxl_lib.found() and jxl_threads_lib.found()}, section: 'Supported image formats')

