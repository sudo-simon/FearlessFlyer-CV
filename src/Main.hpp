#pragma once

#include "modules/BlockingQueue.hpp"
#include "modules/StateBoard.hpp"
#include "modules/Threading/StitcherThread.hpp"
#include <buffers.hpp>
#include <iostream>
#include <opencv2/core/mat.hpp>
#include <stdio.h>
#include <thread>
#include <opencv2/opencv.hpp>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

#include "libs/imgui/imgui.h"
#include "libs/imgui/backends/imgui_impl_glfw.h"
#include "libs/imgui/backends/imgui_impl_opengl3.h"

#include "modules/Console/Console.hpp"
#include "modules/Network/NetConf.hpp"
#include "modules/Threading/CaptureThread.hpp"

struct WindowsCheck{
    bool show_demo_window = false;
    bool show_help_window = false;
    bool show_console = false;
    bool serverOn = false;
    bool isCapturing = false;
    bool errorCapturing = false;
};


class WindowsHandler{
    private:
        ImGuiWindowFlags window_flags = 0;

        NetConf network;

        FIFOBuffer<cv::Mat> fifo_buffer_cap;
        BlockingQueue<cv::Mat> fifo_buffer_sti;
        BlockingQueue<cv::Mat> mapBuffer;
        StateBoard termSig;

        CaptureThread capturer;
        StitcherThread stitcher;
        std::thread capturerThread;
        std::thread stitcherThread;
        cv::Mat frame;
        cv::Mat map;

        Console myConsole;
        GLFWwindow* window;


        static void ImageViewer(cv::Mat& image)
        {
            GLuint texture;
            glGenTextures( 1, &texture );
            glBindTexture( GL_TEXTURE_2D, texture );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, image.cols, image.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data );
            ImGui::Image( reinterpret_cast<void*>( static_cast<intptr_t>( texture ) ), ImVec2( image.cols, image.rows ) );
        }

    public:

        ImVec4 bg_color = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
        WindowsCheck checks;

        static void glfw_error_callback(int error, const char* description)
        {
            fprintf(stderr, "GLFW Error %d: %s\n", error, description);
        }

        WindowsHandler(GLFWwindow*& window)
        {
            network.BindIp();
            network.SearchBindPort();
            network.RTMPconfig();
            network.BindRtmpLink();

            bg_color = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);

            fifo_buffer_cap = FIFOBuffer<cv::Mat>(8);
            //fifo_buffer_sti =  BlockingQueue<cv::Mat>();
            this->stitcher.InitializeStitcher(&fifo_buffer_sti, &mapBuffer, &termSig);
            this->capturer.InitializeCapturer(network.GetExternalRtmpLink(), &fifo_buffer_cap, &fifo_buffer_sti, &termSig);

            this->window = window;
        }

        void ImGuiCleanup(){
            if(checks.serverOn){
                network.ServerStop();
                myConsole.PrintUI("Server OFF");
                Console::Log("Server OFF - RTMP capture dismissed");
            }
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            glfwDestroyWindow(window);
            glfwTerminate();
        }

        void ShowConsole()
        {
            ImGui::Begin("Console", NULL, window_flags);
            ImGui::Text("%s", myConsole.GetConsoleText().c_str());
            if(ImGui::Button("Clear"))
                myConsole.Clear();
            ImGui::End();
        }

        void CaptureWindow()
        {
            ImGui::Begin("Live");    
            WindowsHandler::ImageViewer(frame);
            ImGui::End();
        }

        void MapWindow()
        {
            if(mapBuffer.changed){
                mapBuffer.take(map);
            }

            window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground;
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::Begin("Map Viewer", NULL, window_flags);    
            WindowsHandler::ImageViewer(map);
            ImGui::End();
        }

        void SettingsWindow()
        {

            ImGui::Begin("Settings");            
            ImGui::Checkbox("Console", &checks.show_console);
            ImGui::Checkbox("Help", &checks.show_help_window);

            if (ImGui::Button("Start nginx server")){
                if(!checks.serverOn){   
                    network.ServerStart();
                    myConsole.PrintUI("Server ON - RTMP Address: "+network.GetExternalRtmpLink());
                    Console::Log("RTMP address:"+network.GetExternalRtmpLink());
                } else {  
                    myConsole.PrintUI("Server already started.");
                    Console::Log("Server already started.");
                }
                checks.serverOn = !checks.serverOn;    
            }

            if(checks.serverOn){
                ImGui::SameLine();                   
                ImGui::Text("ON");
            } else {
                ImGui::SameLine();                            
                ImGui::Text("OFF");
            }

            if(ImGui::Button("Start Capturing")){
                if(checks.serverOn && !checks.isCapturing){
                    checks.isCapturing = true;
                    checks.errorCapturing = false; 
                    
                    //? CAPTURE THREAD START
                    termSig.write(false);
                    capturerThread = std::thread(&CaptureThread::Start, &capturer);
                    stitcherThread = std::thread(&StitcherThread::Start, &stitcher);

                    myConsole.PrintUI("Capturing...");
                    Console::Log("Capturing...");
                } else {
                    if(checks.isCapturing){
                        myConsole.PrintUI("Capture already started");
                        Console::Log("Capture already started");
                    } else {
                        checks.errorCapturing = true; 
                        myConsole.PrintUI("Error: Can't start capturing, the server is off");
                        Console::Log("Error: Can't start capturing, the server is off");
                    }
                }
            }

            //? RENDERING OF THE FRAME TAKEN FROM THE FIFOBUFFER
            if(checks.isCapturing && checks.serverOn){
                fifo_buffer_cap.pop(&frame);
                ImGui::SameLine();                            
                ImGui::Text("Capturing frames.");
            }

            if(checks.errorCapturing){
                ImGui::SameLine();                            
                ImGui::Text("Error");
            }

            if(ImGui::Button("Stop nginx server")){
                if(checks.serverOn){ 


                    termSig.write(true);
                    
                    capturerThread.join();
                    stitcherThread.join();

                    network.ServerStop();

                    myConsole.PrintUI("Server OFF");
                    Console::Log("Server OFF - RTMP capture dismissed");

                } else {   
                    myConsole.PrintUI("Server already stopped.");
                    Console::Log("Server already stopped.");
                }
                checks.isCapturing = false;
                checks.serverOn  = false;
            }

            //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

};