
#include "StitcherThread.hpp"
#include <opencv2/core/mat.hpp>


void StitcherThread::Start(){

    cv::Mat frame;
    while(1){
        this->fromCap_buffer_ptr->pop(&frame);

        StitchingRoutine(frame);
        MapBufferUpdate();
    }
}

void StitcherThread::InitializeStitcher(FIFOBuffer<cv::Mat>* fifo_ptr, BlockingQueue<cv::Mat>* buffer_ptr){
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
    this->mapBuffer_ptr->put(this->map);
}