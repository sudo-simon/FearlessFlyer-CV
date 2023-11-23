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
#include <sys/types.h>
#include <vector>

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
        string RTMP_address;
        //FIFOBuffer<cv::Mat> frame_buffer;

        const char* shmem_name;
        long shmem_size;
        int shmem_frames;
        shared_memory_object shmem;
        mapped_region mem_region;
        cv::Mat* shmem_ptr;

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
            //this->frame_buffer = FIFOBuffer<cv::Mat>(initial_buffer_capacity);

            
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
            //const int KEY_UP = 65362;
            //const int KEY_RIGHT = 65363;
            //const int KEY_DOWN = 65364;
            //const int KEY_LEFT = 65361;

            cout << "---- CAPTURE THREAD STARTED ----" << endl;

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

            //cv::VideoCapture cap = cv::VideoCapture(this->RTMP_address); 
            cv::VideoCapture cap = cv::VideoCapture(0);
            if (!cap.isOpened()) return;

            cv::Mat frame;
            int pressed_key = -1;
            cv::namedWindow(
                "RTMP capture",
                cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO | cv::WINDOW_GUI_EXPANDED
            );
            cv::resizeWindow("RTMP capture", 800, 800);

            FIFOBuffer<cv::Mat> frame_buffer(16);

            // Main loop
            while(1){

                cap >> frame; 
                if (frame.empty()) break; 

                cv::imshow("RTMP capture", frame);
                pressed_key = cv::pollKey();

                switch (pressed_key) {

                    case -1:
                        break;
                    
                    /*
                    case KEY_UP:    //? UP
                        fMsg = FrameMessage(frame, p2b::DIR_UP);
                        this->frame_buffer.push(fMsg);
                        break;
                    
                    case KEY_RIGHT:    //? RIGHT
                        fMsg = FrameMessage(frame, p2b::DIR_RIGHT);
                        this->frame_buffer.push(fMsg);
                        break;
                    
                    case KEY_DOWN:    //? DOWN
                        fMsg = FrameMessage(frame, p2b::DIR_DOWN);
                        this->frame_buffer.push(fMsg);
                        break;
                    
                    case KEY_LEFT:    //? LEFT
                        fMsg = FrameMessage(frame, p2b::DIR_LEFT);
                        this->frame_buffer.push(fMsg);
                        break;
                    */

                    case 'q':
                        goto END;
                        break;
                    
                    default:
                        frame_buffer.push(frame);
                        cout << "CAP_TH: Frame buffer size = " << frame_buffer.getSize() << endl;
                        break;
                }

                pressed_key = -1;

                if (
                    isShmemEmpty(this->shmem_ptr, this->shmem_size) && 
                    frame_buffer.getSize() >= this->shmem_frames
                ){
                    for (int i=0; i<this->shmem_frames; ++i){
                        frame_buffer.pop(&this->shmem_ptr[i]);
                    }
                }
            
            }

            END:
            cv::destroyAllWindows();
            cap.release();
            network.ServerStop();
            cout << "CAP_TH: Stopped" << endl;
        }


};