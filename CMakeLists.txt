cmake_minimum_required(VERSION 3.21)

project(cpp_software_renderer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(MINIFB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/minifb-0.9.3)

set(SR_SOURCES
    src/core/color.cpp
    src/gfx/framebuffer.cpp
    src/gfx/rasterizer.cpp
    src/platform/window.cpp
)

add_library(sr STATIC ${SR_SOURCES})
target_include_directories(sr PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(sr PUBLIC minifb::minifb)
target_compile_options(sr PRIVATE -Wall -Wextra -pedantic)

add_executable(demo examples/demo.cpp)
target_link_libraries(demo PRIVATE sr)
