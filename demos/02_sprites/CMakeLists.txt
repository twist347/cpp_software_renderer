sr_add_demo(demo_02_sprites sprites.cpp)
target_compile_definitions(demo_02_sprites PRIVATE DEMO_TEX_PATH="${CMAKE_CURRENT_BINARY_DIR}/img.png")
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/img.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
