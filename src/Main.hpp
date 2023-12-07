#pragma once

#include "modules/BlockingQueue.hpp"
#include "modules/Threading/StitcherThread.hpp"
#include <buffers.hpp>
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
    bool show_map_viewer = false;
    bool show_capture_viewer = false;
    bool show_help_window = false;
    bool show_console = false;
    bool serverOn = false;
    bool isCapturing = false;
    bool errorCapturing = false;
};


class ImGuiHandler{
    private:

        WindowsCheck checks;

        GLFWwindow* window;
        ImGuiIO& io;
        
        ImVec4 bg_color = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
        ImGuiWindowFlags window_flags = 0;

        NetConf network;

        FIFOBuffer<cv::Mat> fifo_buffer_cap;
        FIFOBuffer<cv::Mat> fifo_buffer_sti;
        BlockingQueue<cv::Mat> mapBuffer;

        CaptureThread capturer;
        StitcherThread stitcher;
        std::thread capturerThread;
        std::thread stitcherThread;
        cv::Mat frame;
        cv::Mat map;

        Console myConsole;

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

        static void glfw_error_callback(int error, const char* description)
        {
            fprintf(stderr, "GLFW Error %d: %s\n", error, description);
        }

    public:

        ImGuiHandler()
        {
            network.BindIp();
            network.SearchBindPort();
            network.RTMPconfig();
            network.BindRtmpLink();

            bg_color = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);

            fifo_buffer_cap = FIFOBuffer<cv::Mat>(8);
            fifo_buffer_sti =  FIFOBuffer<cv::Mat>(8);
            this->stitcher = StitcherThread(&fifo_buffer_sti, &mapBuffer);
            this->capturer = CaptureThread(network.GetExternalRtmpLink(), &fifo_buffer_cap, &fifo_buffer_sti);

            window = glfwCreateWindow(1080, 720, "Fearless Flyer", nullptr, nullptr);
            if (window == nullptr){
                //Handling
            }

            io = ImGui::GetIO(); (void)io;
        }

        int InitializeImGui()
        {
            glfwSetErrorCallback(glfw_error_callback);
            if (!glfwInit())
                return 1;

            const char* glsl_version = "#version 130";
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

            // Window and Graphics Context
            glfwMakeContextCurrent(window);
            glfwSwapInterval(1); // Enable vsync

            // Dear ImGui Context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
            //io.ConfigViewportsNoAutoMerge = true;
            //io.ConfigViewportsNoTaskBarIcon = true;

            ImGui::StyleColorsDark();

            // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
            ImGuiStyle& style = ImGui::GetStyle();
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                style.WindowRounding = 0.0f;
                style.Colors[ImGuiCol_WindowBg].w = 1.0f;
            }

            // GLFW and OpenGL platform initialization
            ImGui_ImplGlfw_InitForOpenGL(window, true);
            ImGui_ImplOpenGL3_Init(glsl_version);


            //io.Fonts->AddFontDefault();
            //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
            //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
            //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
            //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
            //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
            //IM_ASSERT(font != nullptr);

            // START

            //? START  
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

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
            window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
            ImGui::Begin("Live Capture", NULL, window_flags);    
            ImGuiHandler::ImageViewer(frame);
            ImGui::End();
        }

        void MapWindow()
        {
            if(mapBuffer.changed){
                mapBuffer.take(map);
            }
            window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground;
            ImGui::Begin("Map Viewer", NULL, window_flags);    
            ImGuiHandler::ImageViewer(map);
            ImGui::End();
        }

        void SettingsWindows()
        {
            ImGui::Begin("Settings");            
            ImGui::Checkbox("Console", &checks.show_console);
            ImGui::Checkbox("Map Viewer", &checks.show_map_viewer);
            ImGui::Checkbox("Capture Viewer", &checks.show_capture_viewer);
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
                    capturerThread = std::thread(&CaptureThread::start_v2, &capturer);
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


            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

};