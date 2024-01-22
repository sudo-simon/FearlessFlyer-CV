#pragma once

#include "libs/buffers.hpp"
#include "modules/Network/NetConf.hpp"
#include "modules/BlockingQueue.hpp"

#include <boost/interprocess/creation_tags.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <opencv4/opencv2/core/mat.hpp>
#include <p2b/bitmap.hpp>
#include <string>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <sys/types.h>


using namespace std;
using namespace boost::interprocess;


/*
    Struct used to pass the frame to the shared memory
    - int rows
    - int cols
    - int add_direction
*/
typedef struct ShmemFrameMessage{
    int add_direction;
} ShmemFrameMessage;


/*
    Returns true if the last byte of the ptr is not 0.
    Last byte of shared memory is used as a syncronization byte.
*/
bool isShmemFull(uint8_t* shmem_ptr, const long shmem_size);

/*
    Returns true if all bytes are set to 0
*/
bool isShmemEmpty(uint8_t* shmem_ptr, const long shmem_size);

/*
    Sets the firts byte to 255 to notify thread exit
*/
void notifyThreadExit(uint8_t* shmem_ptr);



class CaptureThread {

    private:
        string RTMP_address;
        FIFOBuffer<cv::Mat>* toMain_buffer_ptr;
        BlockingQueue<cv::Mat>* toStitch_buffer_ptr;

    public:

        CaptureThread() {}
        ~CaptureThread() {}

        /*
            Thread start method
        */
        void InitializeCapturer(std::string RTMP_addr, FIFOBuffer<cv::Mat>* main_buffer_ptr, BlockingQueue<cv::Mat>* stitch_buffer_ptr );

        void start_v1();

        void start_v2();

};