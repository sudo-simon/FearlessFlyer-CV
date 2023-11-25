cmake_minimum_required(VERSION 3.20)
project(FF-CV)


set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/")
set(LIB_DIR "${SRC_DIR}/libs/")
set(MOD_DIR "${SRC_DIR}/modules/")


#~~~~~ JUST COMPILE THE ONES YOU NEED, NOT ALL OF THEM
file(GLOB IMGUI_SRC 
    "${LIB_DIR}/imgui/*.cpp" 
    "${LIB_DIR}/imgui/backends/imgui_impl_opengl3.cpp"
    "${LIB_DIR}/imgui/backends/imgui_impl_glfw.cpp"
)
add_library(imgui STATIC ${IMGUI_SRC})


file(GLOB P2B_SRC "${LIB_DIR}/p2b/*.cpp")
add_library(p2b STATIC ${P2B_SRC})


find_package(OpenCV REQUIRED)
set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++20")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")


file(GLOB SOURCE_FILES 
    "${SRC_DIR}/Main.cpp"
    "${MOD_DIR}/GUI/*.cpp"
    "${MOD_DIR}/Network/*.cpp"
    "${MOD_DIR}/Console/*.cpp"
    "${MOD_DIR}/Threading/CaptureThread.cpp" #commentabile
    "${MOD_DIR}/Threading/PathfinderThread.cpp"
    "${MOD_DIR}/Threading/RecognizerThread.cpp"
    "${MOD_DIR}/Threading/StitcherThread.cpp"
)
add_executable(FF-CV ${SOURCE_FILES})


include_directories(SYSTEM ${LIB_DIR})
include_directories(${SRC_DIR})

#~~~~~ May be needed, most likely are
include_directories(SYSTEM "/usr/local/include/opencv4/")
include_directories(SYSTEM "${LIB_DIR}/boost_1_83_0/")


#~~~~~ WATCH OUT FOR CORRECT NAMES OF LIBRARIES LOCATIONS
target_link_libraries(FF-CV ${OpenCV_LIBS})
target_link_libraries(FF-CV OpenGL::GL)
target_link_libraries(FF-CV glfw)
target_link_libraries(FF-CV imgui)
target_link_libraries(FF-CV p2b)


#~~~~~ Test binary
#add_executable(Test "${SRC_DIR}/Test.cpp" "${MOD_DIR}/Threading/CaptureThread.cpp")
#target_link_libraries(Test ${OpenCV_LIBS})
#target_link_libraries(Test OpenGL::GL)
#target_link_libraries(Test glfw)
#target_link_libraries(Test imgui)
#target_link_libraries(Test p2b)