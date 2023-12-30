project('VulkanLib', 'cpp',
  license: 'GPL3',
        version: '0.1.0',
        meson_version: '>=0.51',
  default_options: ['cpp_std=c++1z'])

cmake = import('cmake')

VulkanAPI = dependency('vulkan')
Shaderc = cmake.subproject('lib/shaderc').dependency()
GLFW = cmake.subproject('lib/glfw').dependency()

headers = include_directories('src/cxx')

project_target = shared_library(
  meson.project_name(),
  dependencies:[VulkanAPI, Shaderc, GLFW] ,
  install : true,
  gnu_symbol_visibility : 'hidden',
  include_directories : headers,
)
