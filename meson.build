project('VulkanLib', 'cpp',
  license: 'GPL3',
        version: '0.1.0',
        meson_version: '>=0.51',
  default_options: ['cpp_std=c++1z'])

cmake = import('cmake')

VulkanAPI = dependency('vulkan')
GLFW = cmake.subproject('glfw').dependency('glfw')
GLM = cmake.subproject('glm').dependency('glm')
headers = include_directories('src/cxx')

project_target = static_library(
  meson.project_name(),
  dependencies:[VulkanAPI, GLM, GLFW] ,
  install : true,
  gnu_symbol_visibility : 'hidden',
  include_directories : headers,
)
