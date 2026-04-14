add_executable(demo_02_sprites sprites.cpp)
target_link_libraries(demo_02_sprites PRIVATE sr)
target_compile_options(demo_02_sprites PRIVATE -Wall -Wextra -pedantic)
target_compile_definitions(demo_02_sprites PRIVATE DEMO_TEX_PATH="${CMAKE_CURRENT_BINARY_DIR}/img.png")

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/img.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
