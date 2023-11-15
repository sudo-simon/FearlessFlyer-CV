#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <regex>

class NetConf {

    private:
        std::string serverIP;
        std::string rtmpLink;

    public:

        NetConf(std::string& ip) : serverIP(ip) {}
        ~NetConf() {};


        inline void SetRtmpLink(std::string ip) {serverIP = ip;}
        inline void SetServerIP(std::string ip, std::string key) {rtmpLink = "rtmp://"+ip+":1935/"+key+"/live";}
        inline void SetServerIP(std::string ip) {rtmpLink = "rtmp://"+ip+":1935/live";}
        
        inline std::string GetRtmpLink() const {return rtmpLink;}
        inline std::string GetServerIP() const {return serverIP;}

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