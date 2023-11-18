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
#include <vector>

class NetConf {

    private:
        
        std::string port;
        std::string internalRtmpLink;
        std::string externalRtmpLink;
        std::string externalIp;
        const std::string internalIp = "127.0.0.1";

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

        NetConf() {}
        ~NetConf() {}


        void BindIp();
        void SearchBindPort();
        void BindRtmpLink(std::string key);
        void BindRtmpLink();
        
        inline std::string GetExternalRtmpLink() const {return externalRtmpLink;}
        inline std::string GetExternalIP() const {return externalIp;}
        inline std::string GetInternalRtmpLink() const {return internalRtmpLink;}

        void ServerStart() const;
        void ServerStop() const;
        void ServerStatus() const;
        int RTMPconfig() const;
        

        static std::string exec(const char* cmd){
            std::array<char, 128> buffer;
            std::string result;
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);

            std::string cmd_string = cmd;
            if (!pipe) {
                throw std::runtime_error("popen() failed with "+cmd_string);
            }

            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                result += buffer.data();
            }
            return result;
        }

};