#pragma once
#include "libs/buffers.hpp"
#include "modules/BlockingQueue.hpp"
#include <opencv4/opencv2/core/mat.hpp>

class StitcherThread {

    private:
        FIFOBuffer<cv::Mat>* fromCap_buffer_ptr;
        BlockingQueue<cv::Mat>* mapBuffer_ptr;
        cv::Mat map;


        void StitchingRoutine(cv::Mat& newFrame);
        inline void MapBufferUpdate();

    public:
        StitcherThread(FIFOBuffer<cv::Mat>* fifo_ptr, BlockingQueue<cv::Mat>* buffer_ptr) : fromCap_buffer_ptr(fifo_ptr), mapBuffer_ptr(buffer_ptr){};

        ~StitcherThread() {}

        void Start();

};