cmake_minimum_required(VERSION 3.10)
project(buddy_engine VERSION 0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(PROJECT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/")
set(PROJECT_BINARY_DIR "${PROJECT_SOURCE_DIR}/build/")
configure_file("${PROJECT_INCLUDE_DIR}/buddy_engine_config.hpp.in" "buddy_engine_config.hpp")

add_library(buddy_lib
    src/HelloTriangle.cpp
)
target_include_directories(buddy_lib PUBLIC "${PROJECT_INCLUDE_DIR}")

add_executable(buddy_engine src/HelloTriangle.cpp)
target_include_directories(buddy_engine 
    PUBLIC
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_INCLUDE_DIR}"
)

if(WIN32)
    include("C:/Users/conno/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif(WIN32)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

target_link_libraries(buddy_engine 
    PUBLIC 
        buddy_lib
        glfw
        GPUOpen::VulkanMemoryAllocator
        Vulkan::Vulkan
)


#alt VK_USE_PLATFORM_XCB_KHR VK_USE_PLATFORM_WAYLAND_KHR
target_compile_options(buddy_engine PRIVATE -DDEBUG_EN)