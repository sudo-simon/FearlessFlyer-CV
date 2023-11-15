cmake_minimum_required(VERSION 3.12)
project(NetExample)

find_package(OpenCV REQUIRED)
add_executable(NetExample src/Example.cpp src/NetConf.cpp)

target_link_libraries(NetExample ${OpenCV_LIBS})