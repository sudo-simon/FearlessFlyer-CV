cmake_minimum_required(VERSION 3.5)
project(FF-CV)


set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/")
set(LIB_DIR "${SRC_DIR}/libs/")


#~~~~~ JUST COMPILE THE ONES YOU NEED, NOT ALL OF THEM
file(GLOB IMGUI_SRC 
    "${LIB_DIR}/imgui/*.cpp" 
    "${LIB_DIR}/imgui/backends/imgui_impl_opengl3.cpp"
    "${LIB_DIR}/imgui/backends/imgui_impl_glfw.cpp"
)
add_library(imgui STATIC ${IMGUI_SRC})


find_package(OpenCV REQUIRED)
set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++20")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")


file(GLOB SOURCE_FILES "${SRC_DIR}/main.cpp")
add_executable(FF-CV ${SOURCE_FILES})


include_directories(SYSTEM ${LIB_DIR})
include_directories(${SRC_DIR})


#~~~~~ WATCH OUT FOR CORRECT NAMES OF LIBRARIES LOCATIONS
target_link_libraries(FF-CV ${OpenCV_LIBS})
target_link_libraries(FF-CV OpenGL::GL)
target_link_libraries(FF-CV glfw)
target_link_libraries(FF-CV imgui)
