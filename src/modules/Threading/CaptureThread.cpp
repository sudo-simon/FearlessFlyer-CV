#include "modules/Threading/CaptureThread.hpp"
#include "modules/BlockingQueue.hpp"
#include "modules/Console/Console.hpp"
#include "modules/StateBoard.hpp"

#include <buffers.hpp>
#include <cstdint>
#include <cstring>
#include <exception>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>


// ----------------------------------------------------------------
bool isShmemFull(uint8_t* shmem_ptr, const long shmem_size){
    return (shmem_ptr[shmem_size-1] == 0) ? false : true;
}

bool isShmemEmpty(uint8_t* shmem_ptr, const long shmem_size){
    uint8_t* ptr_copy = shmem_ptr;
    for (long i=0; i<shmem_size; ++i){
        if (*ptr_copy++ != 0) return false;
    }
    return true;
}

void notifyThreadExit(uint8_t* shmem_ptr){
    *shmem_ptr = 255;
}
// ----------------------------------------------------------------

void CaptureThread::InitializeCapturer(std::string RTMP_addr, FIFOBuffer<cv::Mat>* main_buffer_ptr, BlockingQueue<cv::Mat>* stitch_buffer_ptr,  StateBoard* termSig){
    this->RTMP_address = RTMP_addr;
    this->toMain_buffer_ptr = main_buffer_ptr;
    this->toStitch_buffer_ptr = stitch_buffer_ptr;
    this->termSig_ptr = termSig;
}


void CaptureThread::Start(){

    cout << "---- CAPTURE THREAD STARTED ----" << endl;

    cv::Mat frame;
    //cv::VideoCapture cap(0);
    //cv::VideoCapture cap(this->RTMP_address);
    cv::VideoCapture cap("2.mp4");

    if(!cap.isOpened()){
        Console::LogError("VideoCapture() failed");
    }
    
    int framesCounter = 0;


    bool isTerminated = false;
    while(!isTerminated){
        cap >> frame;
        if (frame.empty()) continue;
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGBA);
        this->toMain_buffer_ptr->push(frame);

        if(framesCounter == 30){
            this->toStitch_buffer_ptr->put(frame);
            framesCounter = 0;
        } else {
            framesCounter++;
        }

        this->termSig_ptr->read(isTerminated);
    }

    this->toStitch_buffer_ptr->put(frame);
    cout << "---- CAPTURE THREAD STOPPED ----" << endl;
}