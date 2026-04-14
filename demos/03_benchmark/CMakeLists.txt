add_executable(demo_03_benchmark benchmark.cpp)
target_link_libraries(demo_03_benchmark PRIVATE sr)
target_compile_options(demo_03_benchmark PRIVATE -Wall -Wextra -pedantic)
