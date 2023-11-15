#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <fstream>
#include <opencv2/opencv.hpp>

#include <regex>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>    std::cout << ip << std::endl;


//to do: cercare il local ip per fare l'equal generalizzato, migliorare la regex in modo tale che non prenda stringhe "inet "

void exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);

    std::string cmd_string = cmd;
    if (!pipe) {
        throw std::runtime_error("popen() failed with "+cmd_string);
    }
}

int configSetup(){

    std::ifstream file("/etc/nginx/nginx.conf"); 
    
    std::string toFind = "rtmp"; 
    std::string rtmpConfiguration = "rtmp { " 
	                                "    server {"
		                            "        listen 1935;"
		                            "        chunk_size 4096;"
		                            "        application live {"
			                        "            live on;"
			                        "            record off;"
			                        "            allow publish all;"
			                        "            allow play all;"
		                            "        }"
	                                "    }"
                                    "}";
    
    std::string line;
    bool found = false;
    if (file.is_open()) {
        while (getline(file, line)) {
            if (line.find(toFind) != std::string::npos) {
                std::cout << "RTMP already setup." << std::endl;
                found = true;
                break;
            }
        }
        file.close();


        if (!found) {
            std::ofstream file("/etc/nginx/nginx.conf");
            file << rtmpConfiguration << std::endl;
            std::cout << "RTMP setup done." << std::endl;
        }
    } else {
        std::cerr << "Unable to open file." << std::endl;
    }

    return 0;
}

std::string GetExternalIP(){

    std::array<char, 128> buffer;
    std::string result;

    //pipe is the result of popen: popen is a function that execute shell istructions
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("ifconfig", "r"), pclose);

    if (!pipe) {
        throw std::runtime_error("popen() failed with ifconfig!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    std::smatch matches;
    std::regex reg("(inet )(?:1?[0-9]{1,2}|2[0-4][0-9]|25[0-5]).(?:1?[0-9]{1,2}|2[0-4][0-9]|25[0-5]).(?:1?[0-9]{1,2}|2[0-4][0-9]|25[0-5]).(?:1?[0-9]{1,2}|2[0-4][0-9]|25[0-5])");

    std::vector<std::string> ips;
    while (std::regex_search (result,matches,reg)) {
        for (auto x : matches) 
            ips.push_back(x);

        result = matches.suffix().str();
    }

    std::string external_ip;
    for(std::string ip: ips){
        std::string ip_cleaned = "";
        for(char c : ip){
            if(c > 57 || c < 46){
                continue;
            }
            ip_cleaned += c;
        }

        if(ip_cleaned == "127.0.0.1" || ip_cleaned == ""){
            continue;
        }
        external_ip = ip_cleaned;
    }
    return external_ip;
}

int main(){

    //Installa nginx 

    configSetup();

    try {
        exec("systemctl start nginx");
    } catch (const std::runtime_error& e) {
        std::string error = e.what();
        throw std::runtime_error(error+"\n");
    }
    std::cout << "Server started." << std::endl;

    std::string ip = GetExternalIP();
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