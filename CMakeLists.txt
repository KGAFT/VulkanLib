cmake_minimum_required(VERSION 3.16)
project(VulkanLib)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE DEF_FILES src/cxx/*.hpp)
file(GLOB_RECURSE SRC_FILES src/cxx/*.cpp)

add_library(VulkanLib ${DEF_FILES} ${SRC_FILES})

set_property(TARGET VulkanLib PROPERTY COMPILE_WARNING_AS_ERROR ON)

add_subdirectory(subprojects/glm)



file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/cxx/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include)
file(GLOB_RECURSE UNNECESSARY_CPP ${CMAKE_CURRENT_BINARY_DIR}/include/*.cpp)
file(REMOVE ${UNNECESSARY_CPP})

if(NOT MSVC)
    target_compile_options(VulkanLib PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter )
endif()

if(DEFINED VulkanSDKPath)
    message(STATUS "Using manually defined path to vulkan sdk, if this sdk is not installed in path the validation layers may not be working")
    if(WIN32)
        target_include_directories(VulkanLib PUBLIC ${VulkanSDKPath}/Include)
        target_link_directories(VulkanLib PUBLIC ${VulkanSDKPath}/bin ${VulkanSDKPath}/lib)
        set(VulkanLibList shaderc_shared vulkan-1)
    endif()

    if(UNIX AND NOT APPLE)
        target_include_directories(VulkanLib PUBLIC  ${VulkanSDKPath}/include)
        target_link_directories(VulkanLib PUBLIC ${VulkanSDKPath}/bin ${VulkanSDKPath}/lib)
        set(VulkanLibList vulkan shaderc_shared)
    endif()
    if(APPLE)
        target_include_directories(VulkanLib PUBLIC  ${VulkanSDKPath}/include)
        target_link_directories(VulkanLib PUBLIC ${VulkanSDKPath}/bin ${VulkanSDKPath}/lib)
        set(VulkanLibList vulkan.1 shaderc_shared)
    endif()
    target_link_libraries(VulkanLib PUBLIC ${VulkanLibList} glm )
else ()
    message("Auto detected vulkan")
    find_package(Vulkan REQUIRED shaderc_combined)
    target_link_libraries(VulkanLib PUBLIC Vulkan::Vulkan Vulkan::shaderc_combined glm)
endif ()

target_include_directories(VulkanLib PUBLIC src/cxx)


