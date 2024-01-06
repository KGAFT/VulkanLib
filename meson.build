project('VulkanLib', 'cpp',
  license: 'GPL2',
        version: '0.1.0',
        meson_version: '>=0.51',
  default_options: ['cpp_std=c++1z'])

cmake = import('cmake')

opt_var = cmake.subproject_options()

opt_var.add_cmake_defines({'SDL_INCLUDE_DIR': 'subprojects/SDL/include/SDL3'})

Shaderc = dependency('shaderc')
VulkanAPI = dependency('vulkan')
GLFW = cmake.subproject('glfw').dependency('glfw')
GLM = cmake.subproject('glm').dependency('glm')
SDL = cmake.subproject('SDL').dependency('SDL3-shared')
headers = include_directories('src/cxx')

VulkanLib = static_library(
  meson.project_name(),
    ['src/cxx/VulkanLib/t.cpp'],
  dependencies:[VulkanAPI, GLM, GLFW, Shaderc, SDL] ,
  install : true,
  gnu_symbol_visibility : 'hidden',
  include_directories : headers,
)


VulkanLibDep = declare_dependency(include_directories : headers,
  link_with : VulkanLib)

