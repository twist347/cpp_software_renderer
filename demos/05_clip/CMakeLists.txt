add_executable(demo_05_clip clip.cpp)
target_link_libraries(demo_05_clip PRIVATE sr)
target_compile_options(demo_05_clip PRIVATE -Wall -Wextra -pedantic)
