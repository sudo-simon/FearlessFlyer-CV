#pragma once
#include "libs/buffers.hpp"
#include "modules/BlockingQueue.hpp"
#include "modules/StateBoard.hpp"
#include <opencv4/opencv2/core/mat.hpp>

class StitcherThread {

    private:
        BlockingQueue<cv::Mat>* fromCap_buffer_ptr;
        BlockingQueue<cv::Mat>* mapBuffer_ptr;
        StateBoard* termSig_ptr;
        cv::Mat map;


        void StitchingRoutine(cv::Mat& newFrame);
        inline void MapBufferUpdate();

    public:
        StitcherThread() {}
        ~StitcherThread() {}

        void InitializeStitcher(BlockingQueue<cv::Mat>* fifo_ptr, BlockingQueue<cv::Mat>* buffer_ptr, StateBoard* termSig);
        void Start();
        void Terminate();

};