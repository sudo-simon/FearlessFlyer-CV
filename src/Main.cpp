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
#include "modules/BlockingQueue.hpp"
#include "modules/Threading/CaptureThread.hpp"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static void ImageViewer(cv::Mat& image){
    GLuint texture;
    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, image.cols, image.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data );
    ImGui::Image( reinterpret_cast<void*>( static_cast<intptr_t>( texture ) ), ImVec2( image.cols, image.rows ) );
}

int main() {

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Window and Graphics Context
    GLFWwindow* window = glfwCreateWindow(540, 360, "RTMP", nullptr, nullptr);
    if (window == nullptr)
        return 1;
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
    bool show_demo_window = false;
    bool show_map_viewer = false;
    bool show_capture_viewer = false;
    bool show_help_window = false;
    bool show_console = false;
    bool serverOn = false;
    bool isCapturing = false;
    bool errorCapturing = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImGuiWindowFlags window_flags = 0;

    NetConf network;
    network.BindIp();
    network.SearchBindPort();
    network.RTMPconfig();
    network.BindRtmpLink();

    CaptureThread capturer(network.GetExternalRtmpLink());
    std::thread capturerThread;

    Console myConsole;
    // START

    cv::VideoCapture cap;
    cv::Mat frame;

    // UPDATE
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //WINDOWS
        if(show_console){
            window_flags = 0;
            ImGui::Begin("Console", NULL, window_flags);
            ImGui::Text("%s", myConsole.GetConsoleText().c_str());
            if(ImGui::Button("Clear"))
                myConsole.Clear();
            ImGui::End();
        }

        {
            ImGui::Begin("Settings");            
            ImGui::Checkbox("Console", &show_console);
            ImGui::Checkbox("Map Viewer", &show_map_viewer);
            ImGui::Checkbox("Capture Viewer", &show_capture_viewer);
            ImGui::Checkbox("Help", &show_help_window);

            if (ImGui::Button("Start nginx server")){
                if(!serverOn){   
                    network.ServerStart();
                    myConsole.PrintUI("Server ON - RTMP Address: "+network.GetExternalRtmpLink());
                    Console::Log("RTMP address:"+network.GetExternalRtmpLink());
                } else {  
                    myConsole.PrintUI("Server already started.");
                    Console::Log("Server already started.");
                }
                serverOn = !serverOn;    
            }

            if(serverOn){
                ImGui::SameLine();                   
                ImGui::Text("ON");
            } else {
                ImGui::SameLine();                            
                ImGui::Text("OFF");
            }

            if(ImGui::Button("Start Capturing")){
                if(serverOn && !isCapturing){
                    isCapturing = true;
                    errorCapturing = false; 

                    cap = cv::VideoCapture(network.GetInternalRtmpLink());
                    if(!cap.isOpened()){
                        Console::LogError("VideoCapture() failed");
                    }
                    capturerThread = std::thread(&CaptureThread::start, &capturer);

                    myConsole.PrintUI("Capturing...");
                    Console::Log("Capturing...");
                } else {
                    if(isCapturing){
                        myConsole.PrintUI("Capture already started");
                        Console::Log("Capture already started");
                    } else {
                        errorCapturing = true; 
                        myConsole.PrintUI("Error: Can't start capturing, the server is off");
                        Console::Log("Error: Can't start capturing, the server is off");
                    }
                }
            }

            if(isCapturing && serverOn){
                cap >> frame;
                if(!frame.empty()){
                    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGBA);

                    synch_queue.put(frame);

                    ImGui::SameLine();                            
                    ImGui::Text("Capturing frames.");
                }
            }

            if(errorCapturing){
                ImGui::SameLine();                            
                ImGui::Text("Error");
            }

            if(ImGui::Button("Stop nginx server")){
                if(serverOn){ 
                    network.ServerStop();
                    myConsole.PrintUI("Server OFF");
                    Console::Log("Server OFF - RTMP capture dismissed");
                } else {   
                    myConsole.PrintUI("Server already stopped.");
                    Console::Log("Server already stopped.");
                }
                isCapturing = false;
                serverOn  = false;
            }


            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // if(show_map_viewer){
        //     window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground;
        //     ImGui::Begin("Map Viewer", NULL, window_flags);    
        //     ImageViewer(testImage);
        //     ImGui::End();
        // }

        if(show_capture_viewer && isCapturing){
            window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
            ImGui::Begin("Live Capture", NULL, window_flags);    
            ImageViewer(frame);
            ImGui::End();
        }

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        if(show_help_window)
        {
            static float f = 0.0f;
            ImGui::Begin("Help");
            ImGui::Text("From the ImGui Demo:");              
            ImGui::Checkbox("Demo Window", &show_demo_window); 
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            
            ImGui::ColorEdit3("clear color", (float*)&clear_color); 
            ImGui::End();
        }
        // WINDOWS

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    if(serverOn){
        network.ServerStop();
        myConsole.PrintUI("Server OFF");
        Console::Log("Server OFF - RTMP capture dismissed");
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
