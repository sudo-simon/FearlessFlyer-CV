#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <regex>

class NetConf {

    private:
        std::string serverIp;
        std::string rtmpLink;

    public:

        NetConf(std::string& ip) : serverIp(ip) {}
        ~NetConf() {};


        inline void SetServerIp(std::string ip) {serverIp = ip;}
        inline void BindRtmpLink(std::string key) {rtmpLink = "rtmp://"+serverIp+":1935/live/"+key;}
        inline void BindRtmpLink() {rtmpLink = "rtmp://"+serverIp+":1935/live";}
        
        inline std::string GetRtmpLink() const {return rtmpLink;}
        inline std::string GetServerIP() const {return serverIp;}

        void ServerStart() const;
        void ServerStop() const;
        void ServerStatus() const;
        std::string RTMPconfig() const;
        

        static inline void exec(const char* cmd){
            std::array<char, 128> buffer;
            std::string result;
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);

            std::string cmd_string = cmd;
            if (!pipe) {
                throw std::runtime_error("popen() failed with "+cmd_string);
            }
        }

};