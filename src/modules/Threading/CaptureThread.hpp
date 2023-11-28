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
        FIFOBuffer<cv::Mat> frame_buffer; 
        FIFOBuffer<struct ShmemFrameMessage> msg_buffer;

        BlockingQueue<cv::Mat>* synch_queue;

        FIFOBuffer<cv::Mat>* fifo_buffer_ptr;

        const char* shmem_name;
        size_t shmem_size;
        int shmem_n_frames;
        int frame_height;
        int frame_width;

        shared_memory_object shmem;
        mapped_region mem_region;
        uint8_t* shmem_ptr;

    public:
        
        CaptureThread(BlockingQueue<cv::Mat>* shared_queue, string link);

        CaptureThread(FIFOBuffer<cv::Mat>* fifo_buffer_ptr, std::string rtmp_addr);

        /*
            Constructor
        */
        CaptureThread(
            const long initial_buffer_capacity, 
            const char* shmem_name, 
            const size_t shmem_size,
            const int shmem_n_frames,
            const int frame_height,
            const int frame_width
        );
        
        /*
            Deconstructor
        */
        ~CaptureThread();

        /*
            Thread start method
        */
        void start_v1();

        void start_v2();

        void start_v3();

};