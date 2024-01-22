
#include "StitcherThread.hpp"
#include "modules/BlockingQueue.hpp"
#include <buffers.hpp>
#include <cstddef>
#include <iostream>
#include <opencv2/core/mat.hpp>

#include <opencv2/opencv.hpp>
#include <ostream>


void StitcherThread::Start(){

    cv::Mat frame;
    std::cout << "---- STITCHER THREAD STARTED ----" << std::endl;
    while(1){
        this->fromCap_buffer_ptr->take(frame);
        StitchingRoutine(frame);
        if(!frame.empty()){
            std::cout << "PRINT" << std::endl;
            MapBufferUpdate();
        }
    }
}

void StitcherThread::InitializeStitcher(BlockingQueue<cv::Mat>* fifo_ptr, BlockingQueue<cv::Mat>* buffer_ptr){
    this->fromCap_buffer_ptr = fifo_ptr;
    this->mapBuffer_ptr = buffer_ptr;
}

void StitcherThread::StitchingRoutine(cv::Mat& newFrame){

    //Features extraction from the map and the new frame (ORB or FAST)

    //Features movement extimation (RANSAC)

    // apply the image on the map
    this->map = newFrame;
}


inline void StitcherThread::MapBufferUpdate(){
    //cv::imwrite("test.png", this->map);
    this->mapBuffer_ptr->put(this->map);
}