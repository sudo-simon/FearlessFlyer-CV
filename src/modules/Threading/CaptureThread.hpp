#pragma once

#include "libs/buffers.hpp"
#include "modules/network/NetConf.hpp"

#include <boost/interprocess/creation_tags.hpp>
#include <opencv2/videoio.hpp>
#include <opencv4/opencv2/core/mat.hpp>
#include <p2b/bitmap.hpp>
#include <string>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

using namespace std;
using namespace boost::interprocess;


// Struct used to pass the frame to the BitmapThread class
typedef struct FrameMessage{
    cv::Mat frame;
    int add_direction;
} FrameMessage;



bool isShmemEmpty(FrameMessage* ptr, const long shmem_size){
    char* tmp_cast = (char*) ptr;
    for (long i=0; i<shmem_size; ++i){
        if (*tmp_cast++ != 0) return false;
    }
    return true;
}


class CaptureThread {

    private:
        string RTMP_address;
        FIFOBuffer<FrameMessage> frame_buffer;

        const char* shmem_name;
        long shmem_size;
        int shmem_frames;
        shared_memory_object shmem;
        mapped_region mem_region;
        FrameMessage* shmem_ptr;

    public:
        
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

            //this->RTMP_address = "NULL";
            this->frame_buffer = FIFOBuffer<FrameMessage>(initial_buffer_capacity);
            
            this->shmem_name = shmem_name;
            this->shmem_size = shmem_size;
            this->shmem_frames = shmem_frames;
            this->shmem = shared_memory_object(
                open_only,
                this->shmem_name,
                read_write
            );
            this->mem_region = mapped_region(this->shmem, read_write);
            this->shmem_ptr = (FrameMessage*) this->mem_region.get_address();

        }
        
        // Destructor
        ~CaptureThread(){
            shared_memory_object::remove(this->shmem_name);
        }



        // Thread start method
        void start(){
            cout << "---- CAPTURE THREAD STARTED ----\n" << endl;

            // Network module init
            NetConf network;
            network.BindIp();
            network.SearchBindPort();
            network.RTMPconfig();
            network.ServerStart();
            network.BindRtmpLink("test");
            std::cout << "CAP_TH: RTMP address: " << network.GetExternalRtmpLink()<< std::endl;

            this->RTMP_address = network.GetInternalRtmpLink();

            std::cout << "CAP_TH: Press <ENTER> to start capturing." << std::endl;
            std::cin.ignore();
            std::cout << "CAP_TH: Capturing..." << std::endl;

            cv::VideoCapture cap = cv::VideoCapture(this->RTMP_address); 
            if (!cap.isOpened()) return;

            cv::Mat frame;
            int pressed_key = -1;
            cv::namedWindow("RTMP capture");

            FrameMessage* frame_msg = (FrameMessage*) malloc(sizeof(struct FrameMessage));

            // Main loop
            while(1){

                cap >> frame; 
                if (frame.empty()) break; 

                cv::imshow("RTMP capture", frame);
                pressed_key = cv::pollKey();

                switch (pressed_key) {

                    case -1:
                        break;
                    
                    case 82:    //? UP
                        frame_msg->frame = frame;
                        frame_msg->add_direction = p2b::DIR_UP;
                        frame_buffer.push(*frame_msg);
                        break;
                    
                    case 83:    //? RIGHT
                        frame_msg->frame = frame;
                        frame_msg->add_direction = p2b::DIR_RIGHT;
                        frame_buffer.push(*frame_msg);
                        break;
                    
                    case 84:    //? DOWN
                        frame_msg->frame = frame;
                        frame_msg->add_direction = p2b::DIR_DOWN;
                        frame_buffer.push(*frame_msg);
                        break;
                    
                    case 81:    //? LEFT
                        frame_msg->frame = frame;
                        frame_msg->add_direction = p2b::DIR_LEFT;
                        frame_buffer.push(*frame_msg);
                        break;
                    
                    default:
                        cout << "CAP_TH: Pressed key = " << pressed_key << endl;
                        break;
                }

                pressed_key = -1;

                if (
                    isShmemEmpty(this->shmem_ptr, this->shmem_size) && 
                    this->frame_buffer.getSize() >= this->shmem_frames
                ){
                    for (int i=0; i<this->shmem_frames; ++i){
                        this->frame_buffer.pop(&this->shmem_ptr[i]);
                    }
                }
            
            }

            free(frame_msg);
            cv::destroyAllWindows();
            cap.release();
            network.ServerStop();
        }


};