cmake_minimum_required(VERSION 3.5)
project(VulkanLib)

find_package(Vulkan REQUIRED shaderc_combined)
set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE H_FILES src/cxx/*.hpp)
file(GLOB_RECURSE CPP_FILES src/cxx/*.cpp)

add_library(VulkanLib ${H_FILES} ${CPP_FILES})

add_subdirectory(lib/glfw)
add_subdirectory(lib/glm)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/cxx/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/glsl/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(GLOB_RECURSE UNNECESSARY_CPP ${CMAKE_CURRENT_BINARY_DIR}/include/*.cpp)
file(REMOVE ${UNNECESSARY_CPP})


target_link_libraries(VulkanLib PUBLIC Vulkan::Vulkan Vulkan::shaderc_combined glfw glm)
target_include_directories(VulkanLib PUBLIC src/cxx)

file(GLOB_RECURSE EXEC_SRC_FILES src/exec/*.cpp)
file(GLOB_RECURSE EXEC_DEF_FILES src/exec/*.hpp)


add_executable(VulkanExec ${EXEC_SRC_FILES} ${EXEC_DEF_FILES})

target_link_libraries(VulkanExec PUBLIC VulkanLib)

