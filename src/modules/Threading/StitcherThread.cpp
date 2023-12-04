
#include "StitcherThread.hpp"
#include <opencv2/core/mat.hpp>


void StitcherThread::Start(){

    cv::Mat frame;
    (*this->fifo_buffer_ptr).pop(&frame);

    StitchingRoutine(frame);
    MapBufferUpdate();
}


void StitcherThread::StitchingRoutine(cv::Mat& newFrame){

    //Features extraction from the map and the new frame (ORB or FAST)

    //Features movement extimation (RANSAC)

    // apply the image on the map
}


inline void StitcherThread::MapBufferUpdate(){
    (*this->mapBuffer_ptr).put(this->map);
}