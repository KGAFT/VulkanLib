project('VulkanLib', 'cpp',
  license: 'GPL2',
        version: '0.1.0',
        meson_version: '>=0.51',
  default_options: ['cpp_std=c++1z'])

cmake = import('cmake')
Shaderc = dependency('shaderc')
VulkanAPI = dependency('vulkan')
GLFW = cmake.subproject('glfw').dependency('glfw')
GLM = cmake.subproject('glm').dependency('glm')
headers = include_directories('src/cxx')

VulkanLib = static_library(
  meson.project_name(),
    ['src/cxx/VulkanLib/t.cpp'],
  dependencies:[VulkanAPI, GLM, GLFW, Shaderc] ,
  install : true,
  gnu_symbol_visibility : 'hidden',
  include_directories : headers,
)


VulkanLibDep = declare_dependency(include_directories : headers,
  link_with : VulkanLib)
