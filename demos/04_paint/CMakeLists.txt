add_executable(demo_04_paint paint.cpp)
target_link_libraries(demo_04_paint PRIVATE sr)
target_compile_options(demo_04_paint PRIVATE -Wall -Wextra -pedantic)
