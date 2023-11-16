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

int main(){

    // Installa nginx

    std::string ip;
    std::cout << "Network IP address: ";
    std::cin >> ip; 

    NetConf network(ip);
    network.SearchBindPort();
    std::cout << network.RTMPconfig() << std::endl;
    network.ServerStart();
    network.BindRtmpLink("test");
    std::cout << "RTMP address: " << network.GetRtmpLink()<< std::endl;


    std::cout << "Press <ENTER> to start capturing." << std::endl;
    std::cin.ignore();
    std::cin.ignore();
    std::cout << "Capturing..." << std::endl;

    // rtmp://192.168.1.123:1935/live/test
    int res = VideoStream(network.GetRtmpLink());
    if(res == -1){
        std::cerr << "VideoStream() ERROR." << std::endl;
    }

    network.ServerStop();
}

