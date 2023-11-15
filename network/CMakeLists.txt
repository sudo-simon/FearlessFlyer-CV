cmake_minimum_required(VERSION 3.12)
project(drone_map)

find_package(OpenCV REQUIRED)
add_executable(drone src/framesBuffer.cpp)

target_link_libraries(drone ${OpenCV_LIBS})