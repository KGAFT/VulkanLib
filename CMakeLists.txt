cmake_minimum_required(VERSION 3.6)
project(VulkanLib)

find_package(Vulkan REQUIRED shaderc_combined)
set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE DEF_FILES src/cxx/*.hpp)
file(GLOB_RECURSE SRC_FILES src/cxx/*.cpp)

add_library(VulkanLib ${DEF_FILES} ${SRC_FILES})

set_property(TARGET VulkanLib PROPERTY COMPILE_WARNING_AS_ERROR ON)

add_subdirectory(subprojects/glfw)
add_subdirectory(subprojects/glm)


find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/cxx/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include)
file(GLOB_RECURSE UNNECESSARY_CPP ${CMAKE_CURRENT_BINARY_DIR}/include/*.cpp)
file(REMOVE ${UNNECESSARY_CPP})

if(MSVC)
    target_compile_options(VulkanLib PRIVATE /W4 /WX)
else()
    target_compile_options(VulkanLib PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter)
endif()

target_link_libraries(VulkanLib PUBLIC Vulkan::Vulkan Vulkan::shaderc_combined glfw glm SDL3::SDL3)
target_include_directories(VulkanLib PUBLIC src/cxx)


