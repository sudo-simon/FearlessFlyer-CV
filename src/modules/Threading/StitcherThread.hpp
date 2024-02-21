#pragma once
#include "libs/buffers.hpp"
#include "modules/BlockingQueue.hpp"
#include "modules/StateBoard.hpp"
#include <opencv2/core/types.hpp>
#include <opencv4/opencv2/core/mat.hpp>

class StitcherThread {

    private:
        FIFOBuffer<cv::Mat>* fromCap_buffer_ptr;
        BlockingQueue<cv::Mat>* mapBuffer_ptr;
        StateBoard* termSig_ptr;
    
        cv::Mat map;
        cv::Mat lastFrame;
        float threshOrb;
        float threshRansac;


        void StitchingRoutine(cv::Mat& newFrame);
        inline void MapBufferUpdate();
        cv::Mat warpPerspectiveNoCut(const cv::Mat& srcImage, cv::Mat transformationMatrix);

    public:
        StitcherThread() {}
        ~StitcherThread() {}


        inline void setTresholds(float& orb, float ransac) { this->threshOrb = orb; this->threshRansac = ransac;};

        void InitializeStitcher(FIFOBuffer<cv::Mat>* fifo_ptr, BlockingQueue<cv::Mat>* buffer_ptr, StateBoard* termSig);
        void Start();
        void Terminate();


};