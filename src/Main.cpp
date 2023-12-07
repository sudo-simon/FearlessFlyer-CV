#include "Main.hpp"



int main() {

    ImGuiHandler handler;


    //? UPDATE
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //WINDOWS
        if(show_console){
            handler.ShowConsole();
        }

        {
            handler.SettingsWindows(checks);
        }

        if(show_map_viewer){
            handler.MapWindow(window_flags, mapBuffer, map);
        }

        if(show_capture_viewer && isCapturing){
            handler.CaptureWindow(window_flags,frame);
        }

        if(show_help_window)
        {
            ImGui::ShowDemoWindow(&show_demo_window);
        }
        // WINDOWS

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(bg_color.x * bg_color.w, bg_color.y * bg_color.w, bg_color.z * bg_color.w, bg_color.w);
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
