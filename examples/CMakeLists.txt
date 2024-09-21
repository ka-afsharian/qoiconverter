cmake_minimum_required(VERSION 3.10)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#find_package(Qt5 REQUIRED Widgets)
#find_package(SDL2)

# Add executable
add_executable(example1 example1.cpp)
#target_compile_options(example1 PRIVATE -mavx2 -ftree-vectorize -O3)
target_link_libraries(example1 PRIVATE LIBRARYHERE)
#target_link_libraries(example1 PRIVATE ${SDL2_LIBRARIES})

# Add executable
add_executable(example1_static example1.cpp)
#target_compile_options(example1_static PRIVATE -mavx2 -ftree-vectorize)
target_link_libraries(example1_static PRIVATE LIBRARYHERE)
#target_link_libraries(example1_static PRIVATE ${SDL2_LIBRARIES})
