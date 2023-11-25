#include "modules/Threading/CaptureThread.hpp"
#include "modules/Console/Console.hpp"

#include <cstdint>
#include <cstring>
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


CaptureThread::CaptureThread(BlockingQueue<cv::Mat>* shared_queue){
    this->synch_queue = shared_queue;
}


CaptureThread::CaptureThread(
    const long initial_buffer_capacity, 
    const char* shmem_name, 
    const size_t shmem_size,
    const int shmem_n_frames,
    const int frame_height,
    const int frame_width
){

    if (initial_buffer_capacity <= 0 || shmem_size <= 0){
        cerr << "CAPTURE_THREAD: Unable to create CaptureThread object" << endl;
        exit(1);
    }

    this->RTMP_address = "NULL";

    this->frame_buffer = FIFOBuffer<cv::Mat>(initial_buffer_capacity);
    this->msg_buffer = FIFOBuffer<struct ShmemFrameMessage>(initial_buffer_capacity);
    
    this->shmem_name = shmem_name;
    this->shmem_size = shmem_size;
    this->shmem_n_frames = shmem_n_frames;
    this->frame_height = frame_height;
    this->frame_width = frame_width;

    this->shmem = shared_memory_object(
        open_only,
        this->shmem_name,
        read_write
    );
    this->mem_region = mapped_region(this->shmem, read_write);
    this->shmem_ptr = (uint8_t*) this->mem_region.get_address();

}

CaptureThread::~CaptureThread(){
    shared_memory_object::remove(this->shmem_name);
}



void CaptureThread::start(){

    //? Some constants definitions --------
    const int KEY_UP = 65362;
    const int KEY_RIGHT = 65363;
    const int KEY_DOWN = 65364;
    const int KEY_LEFT = 65361;
    //? -----------------------------------

    cout << "---- CAPTURE THREAD STARTED ----" << endl;

    //? Network module init
    NetConf network;
    network.BindIp();
    network.SearchBindPort();
    network.RTMPconfig();
    network.ServerStart();
    network.BindRtmpLink();
    
    std::cout << "CAPTURE_THREAD: RTMP address: " << network.GetExternalRtmpLink()<< std::endl;
    this->RTMP_address = network.GetExternalRtmpLink();

    std::cout << "CAPTURE_THREAD: Press <ENTER> to start capturing." << std::endl;
    std::cin.ignore();
    std::cout << "CAPTURE_THREAD: Capturing..." << std::endl;

    //? Initializing the OpenCV video capture stream
    cv::VideoCapture cap = cv::VideoCapture(0); //! Line used to capture directly from webcam, DEBUG
    //cv::VideoCapture cap = cv::VideoCapture(this->RTMP_address); 
    if (!cap.isOpened()){
        cerr << "CAPTURE_THREAD: error opening the video capture stream" << endl;
        notifyThreadExit(this->shmem_ptr);
        return;
    }

    //? Getting the resolution of the video capture
    const int FRAME_HEIGHT = (int) cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    const int FRAME_WIDTH = (int) cap.get(cv::CAP_PROP_FRAME_WIDTH);
    const int FRAME_SIZE = FRAME_HEIGHT * FRAME_WIDTH;
    const int TOTAL_SHMEM_MSG_SIZE = sizeof(struct ShmemFrameMessage) + FRAME_SIZE;

    if (FRAME_HEIGHT != this->frame_height || FRAME_WIDTH != this->frame_width){
        cerr << 
            "CAPTURE_THREAD: capture resolution mismatch with initialized frame sizes\n"
            "Passed sizes: "<< this->frame_width << " x " << this->frame_height << "\n"
            "Captured sizes: "<< FRAME_WIDTH << " x " << FRAME_HEIGHT 
        << endl;
        notifyThreadExit(this->shmem_ptr);
        return;
    }

    //? Initialized empty objects
    cv::Mat frame;
    cv::Mat gs_frame;
    int pressed_key = -1;
    
    //? ShmemFrameMessage struct
    ShmemFrameMessage* shmMsg = new ShmemFrameMessage;

    //? Window creation
    cv::namedWindow(
        "RTMP capture",
        cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO | cv::WINDOW_GUI_EXPANDED
    );
    cv::resizeWindow("RTMP capture", 800, 800);

    //? Main loop
    size_t shmem_written_bytes = 0;
    uint8_t* tmp_memory = (uint8_t*) malloc(this->shmem_size);
    cv::Mat tmp_frame;
    ShmemFrameMessage tmp_msg;
    while(1){

        cap >> frame; 
        if (frame.empty()) break; 

        cv::imshow("RTMP capture", frame);
        pressed_key = cv::pollKey();

        switch (pressed_key) {

            //case -1:    //? Nothing pressed
            //    break;
            
            case KEY_UP:    //? UP
                cv::cvtColor(frame, gs_frame, cv::COLOR_BGR2GRAY);

                shmMsg->add_direction = p2b::DIR_UP;
                this->frame_buffer.push(gs_frame.clone());
                this->msg_buffer.push(*shmMsg);

                break;
            
            case KEY_RIGHT:    //? RIGHT
                cv::cvtColor(frame, gs_frame, cv::COLOR_BGR2GRAY);

                shmMsg->add_direction = p2b::DIR_RIGHT;
                this->frame_buffer.push(gs_frame.clone());
                this->msg_buffer.push(*shmMsg);

                break;
            
            case KEY_DOWN:    //? DOWN
                cv::cvtColor(frame, gs_frame, cv::COLOR_BGR2GRAY);

                shmMsg->add_direction = p2b::DIR_DOWN;
                this->frame_buffer.push(gs_frame.clone());
                this->msg_buffer.push(*shmMsg);

                break;
            
            case KEY_LEFT:    //? LEFT
                cv::cvtColor(frame, gs_frame, cv::COLOR_BGR2GRAY);

                shmMsg->add_direction = p2b::DIR_LEFT;
                this->frame_buffer.push(gs_frame.clone());
                this->msg_buffer.push(*shmMsg);

                break;

            case 'q':   //? Quit
                goto END;
                break;

            //default:
            //    break;

        }

        if (
            this->frame_buffer.getSize() >= this->shmem_n_frames &&
            isShmemEmpty(this->shmem_ptr, this->shmem_size)
        ){

            for (int i=0; i<this->shmem_n_frames; ++i){
                this->frame_buffer.pop(&tmp_frame);
                this->msg_buffer.pop(&tmp_msg);
                std::memcpy(
                    tmp_memory+shmem_written_bytes, 
                    &tmp_msg, 
                    sizeof(struct ShmemFrameMessage)
                );
                std::memcpy(
                    tmp_memory+shmem_written_bytes+sizeof(struct ShmemFrameMessage), 
                    tmp_frame.data, 
                    FRAME_SIZE
                );
                shmem_written_bytes += TOTAL_SHMEM_MSG_SIZE;
            }

            std::memcpy(
                this->shmem_ptr, 
                tmp_memory, 
                this->shmem_size
            );
            shmem_written_bytes = 0;
        }
    
    }

    END:

    //? Routine to communicate the capture thread has been stopped via the shmem
    memset(this->shmem_ptr, 0, this->shmem_size);
    notifyThreadExit(this->shmem_ptr);

    free(tmp_memory);

    cv::destroyAllWindows();
    cap.release();
    network.ServerStop();
    cout << "CAPTURE_THREAD: Stopped" << endl;

}




void CaptureThread::start_v2(){

    cout << "---- CAPTURE THREAD STARTED ----" << endl;

    cv::Mat frame;
    int frame_count = 0;

    // Main loop
    while(1){
        (*synch_queue).take(frame);
        if (frame.empty()) continue; 

        if(frame_count < 60){
            frame_count++;
        } else {
            Console::Log("Frame selected");
            cv::imwrite("palle.png", frame);
            frame_count = 0;
        }
    }

    cout << "CAPTURE_THREAD: Stopped" << endl;

}
