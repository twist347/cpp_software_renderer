add_executable(demo_01_primitives primitives.cpp)
target_link_libraries(demo_01_primitives PRIVATE sr)
target_compile_options(demo_01_primitives PRIVATE -Wall -Wextra -pedantic)

