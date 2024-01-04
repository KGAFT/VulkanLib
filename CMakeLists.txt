cmake_minimum_required(VERSION 3.5)
project(VulkanLib)

find_package(Vulkan REQUIRED shaderc_combined)
set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE DEF_FILES src/cxx/*.hpp)
file(GLOB_RECURSE SRC_FILES src/cxx/*.cpp)

add_library(VulkanLib ${DEF_FILES} ${SRC_FILES})

add_subdirectory(subprojects/glfw)
add_subdirectory(subprojects/glm)
add_subdirectory(subprojects/SDL)

set(SDL_INCLUDE_DIR subprojects/SDL/include/SDL3)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/cxx/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include)
file(GLOB_RECURSE UNNECESSARY_CPP ${CMAKE_CURRENT_BINARY_DIR}/include/*.cpp)
file(REMOVE ${UNNECESSARY_CPP})


target_link_libraries(VulkanLib PUBLIC Vulkan::Vulkan Vulkan::shaderc_combined glfw glm SDL3-static)
target_include_directories(VulkanLib PUBLIC src/cxx)


