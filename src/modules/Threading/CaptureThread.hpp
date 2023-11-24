#pragma once

#include "libs/buffers.hpp"
#include "modules/Console/Console.hpp"

#include <boost/interprocess/creation_tags.hpp>
#include <opencv2/videoio.hpp>
#include <opencv4/opencv2/core/mat.hpp>
#include <p2b/bitmap.hpp>
#include <string>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <sys/types.h>
#include <vector>
#include "modules/BlockingQueue.hpp"

using namespace std;
using namespace boost::interprocess;


// Struct used to pass the frame to the BitmapThread class
class FrameMessage{
    public:
        cv::Mat frame;
        int add_direction;
        FrameMessage(){}
        FrameMessage(cv::Mat frame, int add_direction){
            this->frame = frame;
            this->add_direction = add_direction;
        }
};


//? AUX function to check if all shmem is set to 0
bool isShmemEmpty(cv::Mat* ptr, const long shmem_size);


class CaptureThread {

    private:
        //FIFOBuffer<cv::Mat> frame_buffer;
        string rtmpLink;

        // Shared Memory options
        const char* shmem_name;
        long shmem_size;
        int shmem_frames;
        shared_memory_object shmem;
        mapped_region mem_region;
        cv::Mat* shmem_ptr;

    public:

        CaptureThread(std::string link){
            this->rtmpLink = link;
        }
        
        // Constructor
        CaptureThread(
            const long initial_buffer_capacity, 
            const char* shmem_name, 
            const long shmem_size,
            const int shmem_frames
        ){
            if (initial_buffer_capacity <= 0 || shmem_size <= 0){
                cerr << "CAP_TH: Unable to create CaptureThread object" << endl;
                exit(1);
            }

            //this->frame_buffer = FIFOBuffer<cv::Mat>(initial_buffer_capacity);

            //Shared Memory
            this->shmem_name = shmem_name;
            this->shmem_size = shmem_size;
            this->shmem_frames = shmem_frames;
            this->shmem = shared_memory_object(
                open_only,
                this->shmem_name,
                read_write
            );
            this->mem_region = mapped_region(this->shmem, read_write);
            this->shmem_ptr = (cv::Mat*) this->mem_region.get_address();

        }
        
        // Destructor
        ~CaptureThread(){
            shared_memory_object::remove(this->shmem_name);
        }

        // Thread start method
        void start(){

            //? Why do these change depending on the application?
            // const int KEY_UP = 65362;
            // const int KEY_RIGHT = 65363;
            // const int KEY_DOWN = 65364;
            // const int KEY_LEFT = 65361;


            // int pressed_key = -1;

            //FIFOBuffer<cv::Mat> frame_buffer(16);

            cv::Mat frame;
            int frame_count = 0;
            Console::Log("Inizio");

            // Main loop
            while(1){
                synch_queue.take(frame);
                if (frame.empty()) continue; 

                if(frame_count < 60){
                    frame_count++;
                } else {
                    Console::Log("Frame selected");
                    cv::imwrite("palle.png", frame);
                    frame_count = 0;
                }
            }
        }

};