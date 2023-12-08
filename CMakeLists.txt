cmake_minimum_required(VERSION 3.5)
project(VulkanLib)

find_package(Vulkan REQUIRED shaderc_combined)
set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE DEF_FILES src/cxx/*.hpp)
file(GLOB_RECURSE SRC_FILES src/cxx/*.cpp)

add_library(VulkanLib ${DEF_FILES} ${SRC_FILES})

add_subdirectory(lib/glfw)
add_subdirectory(lib/glm)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/cxx/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include)
file(GLOB_RECURSE UNNECESSARY_CPP ${CMAKE_CURRENT_BINARY_DIR}/include/*.cpp)
file(REMOVE ${UNNECESSARY_CPP})


target_link_libraries(VulkanLib PUBLIC Vulkan::Vulkan C:/VulkanSDK/1.3.268.0/Lib/shaderc_sharedd.lib glfw glm)
target_include_directories(VulkanLib PUBLIC src/cxx)


