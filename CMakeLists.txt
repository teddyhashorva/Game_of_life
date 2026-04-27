cmake_minimum_required(VERSION 3.27)

# set cuda patched include directory
set(CUDA_PATCHED_INCLUDE_DIR "$ENV{CUDA_PATCHED_INCLUDE}")
set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -target-dir=$ENV{CUDA_TARGET_DIR_REL} -I${CUDA_PATCHED_INCLUDE_DIR} -L$ENV{CUDA_TOOLKIT_TARGET_LIB_DIR} -L$ENV{CUDA_TOOLKIT_TARGET_LIB_DIR}/stubs")

project(GameOfLife LANGUAGES CXX CUDA)

# optional: define USE_CUDA for conditional compilation (#ifdef USE_CUDA ... #endif)
add_compile_definitions(USE_CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fopenmp")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fopenmp")
include_directories(include)

# ensure patched CUDA headers are included first
include_directories(BEFORE ${CUDA_PATCHED_INCLUDE_DIR})

add_executable(GameOfLife
    src/Main.cpp
    src/Game.cpp
    src/Cli.cpp
    src/World.cu    # CUDA source file
)

target_compile_definitions(GameOfLife PRIVATE USE_CUDA)

# set properties for CUDA separable compilation
set_target_properties(GameOfLife PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
    POSITION_INDEPENDENT_CODE ON
)

# --- OPENMP LINKING FIX ---
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    target_link_libraries(GameOfLife PUBLIC OpenMP::OpenMP_CXX)
endif()
