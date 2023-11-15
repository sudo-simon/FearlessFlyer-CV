cmake_minimum_required(VERSION 3.5)
project(FearlessFlyer-CV)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/")
set(LIBS_DIR "${SRC_DIR}/libs/")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++20")
#add_compile_options(-O2)
#add_compile_options(-g)

file(GLOB SOURCE_FILES "${SRC_DIR}/*.cpp")
add_executable(FF-CV ${SOURCE_FILES})

include_directories(${SRC_DIR})
