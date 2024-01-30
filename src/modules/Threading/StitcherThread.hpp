#pragma once
#include "libs/buffers.hpp"
#include "modules/BlockingQueue.hpp"
#include "modules/StateBoard.hpp"
#include <opencv2/core/types.hpp>
#include <opencv4/opencv2/core/mat.hpp>

class StitcherThread {

    private:
        BlockingQueue<cv::Mat>* fromCap_buffer_ptr;
        BlockingQueue<cv::Mat>* mapBuffer_ptr;
        StateBoard* termSig_ptr;
        cv::Mat map;
        cv::Mat lastFrame;
        cv::Mat lastMatrix;
        cv::Mat lastDescriptor;
        std::vector<cv::KeyPoint> lastKeypoints;


        void StitchingRoutine(cv::Mat& newFrame);
        inline void MapBufferUpdate();
        cv::Mat warpPerspectiveNoCut(const cv::Mat& srcImage, const cv::Mat& transformationMatrix);
        cv::Mat numpyDot(cv::Mat A, cv::Mat B);

    public:
        StitcherThread() {}
        ~StitcherThread() {}

        void InitializeStitcher(BlockingQueue<cv::Mat>* fifo_ptr, BlockingQueue<cv::Mat>* buffer_ptr, StateBoard* termSig);
        void Start();
        void Terminate();


};