cmake_minimum_required(VERSION 3.12)

set(TARGET slitherlink)
set(SRCS slitherlink.cpp)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)

add_executable(${TARGET} ${SRCS})
target_compile_options(${TARGET} PUBLIC -O2 -g -Wall -Wextra -Wpedantic)
target_include_directories(${TARGET} PUBLIC ${PROJECT_SOURCE_DIR}/include)
