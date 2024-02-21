#pragma once

#include "libs/buffers.hpp"
#include "modules/Network/NetConf.hpp"
#include "modules/BlockingQueue.hpp"
#include "modules/StateBoard.hpp"

#include <opencv4/opencv2/core/mat.hpp>
#include <p2b/bitmap.hpp>
#include <string>

#include <sys/types.h>


using namespace std;


class CaptureThread {

    private:
        string RTMP_address;
        BlockingQueue<cv::Mat>* toMain_buffer_ptr;
        BlockingQueue<cv::Mat>* toStitch_buffer_ptr;
        StateBoard* termSig_ptr;

        int countPicker;

    public:

        CaptureThread() {}
        ~CaptureThread() {}


        void InitializeCapturer(std::string RTMP_addr, BlockingQueue<cv::Mat>* main_buffer_ptr, BlockingQueue<cv::Mat>* stitch_buffer_ptr, StateBoard* termSig);

        void setCountPicker(int count){ this->countPicker = count;};

        void Start();

        void Terminate();

};