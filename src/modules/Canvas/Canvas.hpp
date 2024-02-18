#pragma once

#include <opencv2/core/mat.hpp>
#include <buffers.hpp>



class Canvas {

    private:
        cv::Mat canvas;
        long canvas_w, canvas_h;
        
        cv::Mat display;
        const long display_w, display_h;
        long left_border, top_border, right_border, bottom_border;
        
        const long window_w, window_h;
        long window_x, window_y;

        FIFOBuffer<cv::Mat>* frameBuffer;

    
    public:

        Canvas(long display_w, long display_h, long window_w, long window_h, FIFOBuffer<cv::Mat>* frameBuffer);

        cv::Mat* getDisplayPtr(){ return &this->display; }
        
        long getWidth(){ return this->canvas_w; }
        long getHeight(){ return this->canvas_h; }

        int resizeCanvas(long new_w, long new_h);

        int moveWindow(long x, long y);

        int stitchFrame();

        int fitCanvasInDisplay();

};