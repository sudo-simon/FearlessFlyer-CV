#pragma once

#include "modules/BlockingQueue.hpp"
#include <opencv2/calib3d.hpp>
#include <opencv2/core/mat.hpp>
#include <buffers.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <tuple>



class Canvas {

    private:
        cv::Mat canvas;
        long canvas_w, canvas_h;
        
        cv::Mat display;
        long display_w, display_h;
        
        long window_w, window_h;
        long window_x, window_y;

        BlockingQueue<cv::Mat>* mapBuffer;

        cv::Mat next_frame;
        long stitched_frames;

        float threshOrb;
        float threshRansac;


    
    public:

        Canvas(long display_w, long display_h, long window_w, long window_h, BlockingQueue<cv::Mat>* buffer, float tOrb, float tRansac);

        inline cv::Mat* getDisplayPtr(){ return &this->display; }
        
        inline long getWidth(){ return this->canvas_w; }
        inline long getHeight(){ return this->canvas_h; }

        int stitchFrame(cv::Mat newFrame);
        
        cv::Mat removeBlackBorders(cv::Mat& srcImg);

        int moveWindow(long x, long y, long newImage_cols, long newImage_rows);

        int resizeCanvas(long top_border, long bottom_border, long left_border, long right_border);

        void updateDisplay();

        void stitch();
        


};

