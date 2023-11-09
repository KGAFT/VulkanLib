cmake_minimum_required(VERSION 3.00)
project(VulkanLib)

find_package(Vulkan REQUIRED shaderc_combined)
set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE H_FILES src/cxx/*.hpp)
file(GLOB_RECURSE CPP_FILES src/cxx/*.cpp)

add_library(VulkanLib ${H_FILES} ${CPP_FILES})
add_subdirectory(lib/glfw)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/cxx/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include)
file(GLOB_RECURSE UNNECESSARY_CPP ${CMAKE_CURRENT_BINARY_DIR}/include/*.cpp)
file(REMOVE ${UNNECESSARY_CPP})

target_link_libraries(VulkanLib PUBLIC Vulkan::Vulkan Vulkan::shaderc_combined glfw)
target_include_directories(VulkanLib PUBLIC src/cxx)

add_executable(VulkanTest src/exec/main.cpp)

target_link_libraries(VulkanTest PUBLIC VulkanLib)

