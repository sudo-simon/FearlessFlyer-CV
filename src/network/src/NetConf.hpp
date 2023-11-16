#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <regex>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>

class NetConf {

    private:
        std::string serverIp;
        std::string serverPort;
        std::string rtmpLink;

        static bool IsPortFree(int port) {
            int sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                std::cerr << "Non è possibile creare la socket" << std::endl;
                return false;
            }

            sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
            addr.sin_port = htons(port);

            if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                close(sockfd);
                return false;
            }

            close(sockfd);
            return true;
        }

    public:

        NetConf(std::string& ip) : serverIp(ip) {}
        ~NetConf() {};


        inline void SetServerIp(std::string ip) {serverIp = ip;}
        inline void BindRtmpLink(std::string key) {rtmpLink = "rtmp://"+serverIp+":"+serverPort+"/live/"+key;}
        inline void BindRtmpLink() {rtmpLink = "rtmp://"+serverIp+":"+serverPort+"/live";}
        void SearchBindPort();
        
        inline std::string GetRtmpLink() const {return rtmpLink;}
        inline std::string GetServerIP() const {return serverIp;}

        void ServerStart() const;
        void ServerStop() const;
        void ServerStatus() const;
        std::string RTMPconfig() const;
        

        static void exec(const char* cmd){
            std::array<char, 128> buffer;
            std::string result;
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);

            std::string cmd_string = cmd;
            if (!pipe) {
                throw std::runtime_error("popen() failed with "+cmd_string);
            }
        }

};