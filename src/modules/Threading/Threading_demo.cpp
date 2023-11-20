#include "modules/network/NetConf.hpp"
#include "libs/buffers.hpp"
#include "libs/p2b/core.hpp"
#include "libs/p2b/bitmap.hpp"

#include <cstdlib>
#include <opencv2/highgui.hpp>
#include <sys/types.h>
#include <thread>

#include <boost/interprocess/shared_memory_object.hpp>

using namespace std;


//? Multithread experiment with FIFOBuffer




int captureThread(std::string rtmpAddress){

    FIFOBuffer<cv::Mat> frame_buffer(32);
    FIFOBuffer<cv::Mat>* frameBuffer_ptr = &frame_buffer;

    FIFOBuffer<int> direction_buffer(32);
    FIFOBuffer<int>* directionBuffer_ptr = &direction_buffer;


    cv::VideoCapture cap(rtmpAddress); 
    if (!cap.isOpened()) { 
        return -1;
    }

    cv::Mat frame;
    int pressed_key = -1;
    cv::namedWindow("RTMP capture");

    while (true) {
        cap >> frame; 
        if (frame.empty())
            break; 

        cv::imshow("RTMP capture", frame);
        pressed_key = cv::pollKey();

        switch (pressed_key) {
            
            case 82:    //? UP
                frame_buffer.push(frame);
                break;
            
            case 83:    //? RIGHT
                break;
            
            case 84:    //? DOWN
                break;
            
            case 81:    //? LEFT
                break;
            
            default:
                cout << "Pressed key = " << pressed_key << endl;
                break;
        }
    }

    //END:
    cv::destroyAllWindows();
    cap.release();

    return 0;

}


int bitmapThread(){

    p2b::Bitmap frame_bitmap(
        800,
        200,
        2,
        {85, 170, 255}
    );

    cv::namedWindow("Bitmap visualization");
    
    return 0;

}













int Threading_demo(){

    // Installa nginx

    NetConf network;
    network.BindIp();
    network.SearchBindPort();
    network.RTMPconfig();
    network.ServerStart();
    network.BindRtmpLink("test");
    std::cout << "RTMP address: " << network.GetExternalRtmpLink()<< std::endl;


    std::cout << "Press <ENTER> to start capturing." << std::endl;
    std::cin.ignore();
    std::cout << "Capturing..." << std::endl;

    thread capture_thread = thread(captureThread,network.GetInternalRtmpLink());
    thread bitmap_thread = thread(bitmapThread);

    int res = capture_thread.join();
    if(res == -1){
        std::cerr << "VideoStream() ERROR." << std::endl;
    }

    network.ServerStop();
    return 0;
}
