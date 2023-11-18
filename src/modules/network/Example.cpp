#include "NetConf.hpp"

int VideoStream(std::string rtmpAddress){

    cv::VideoCapture cap(rtmpAddress); 
    if (!cap.isOpened()) { 
        return -1;
    }

    while (true) {
        cv::Mat frame;
        cap >> frame; 
        if (frame.empty())
            break; 

        cv::imshow("RTMP", frame);

        if (cv::waitKey(30) >= 0){
            break;
        }
    }
    cap.release();
    return 0;
}

int Network_demo(){

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

    int res = VideoStream(network.GetInternalRtmpLink());
    if(res == -1){
        std::cerr << "VideoStream() ERROR." << std::endl;
    }

    network.ServerStop();
    return 0;
}

