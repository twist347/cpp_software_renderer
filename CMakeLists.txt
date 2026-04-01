cmake_minimum_required(VERSION 3.21)

project(cpp_software_renderer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(MINIFB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/minifb-0.9.3)

set(SR_CORE_SOURCES
        src/core/color.cpp
)

set(SR_GFX_SOURCES
        src/gfx/framebuffer.cpp
        src/gfx/rasterizer.cpp
        src/gfx/renderer.cpp
        src/gfx/texture.cpp
)

set(SR_THIRDPARTY_SOURCES
        src/thirdparty/stb_image_impl.cpp
)

set(SR_PLATFORM_SOURCES
        src/platform/window.cpp
)

set(SR_SOURCES
        ${SR_CORE_SOURCES}
        ${SR_GFX_SOURCES}
        ${SR_PLATFORM_SOURCES}
        ${SR_THIRDPARTY_SOURCES}
)

add_library(sr STATIC ${SR_SOURCES})
target_include_directories(sr PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(sr PRIVATE ${PROJECT_SOURCE_DIR}/thirdparty)
target_link_libraries(sr PRIVATE minifb::minifb)
target_compile_options(sr PRIVATE -Wall -Wextra -pedantic)

add_subdirectory(demos)
