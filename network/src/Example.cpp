#include "NetConf.hpp"

int main(){

    //Installa nginx

    std::string ip;
    std::cout << "Network IP address: ";
    std::cin >> ip; 

    NetConf network(ip);
    int res;
    res = network.ServerStart();
    std::cout << "Server started." << std::endl;

    std::string rtmpAddress = "rtmp://"+ip+":1935/live"; 
    std::cout << "RTMP address: " << rtmpAddress << std::endl;

    /*
    cv::VideoCapture cap(rtmpAddress); 
    if (!cap.isOpened()) { 
        return -1;
    }

    cv::Mat frame;
    while (true) {
        cap >> frame; 
        if (frame.empty())
            break; 

        cv::imshow("RTMP", frame);

        if (cv::waitKey(30) >= 0)
            break;
    }

    cap.release();
    */

    try {
        exec("systemctl stop nginx");
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << '\n';
    }
    std::cout << "Server stopped." << std::endl;
}