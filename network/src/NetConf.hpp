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
#include <netdb.h> 




class NetConf {

    private:
        std::string serverIP;
        std::string rtmpLink;

    public:

        NetConf(std::string& ip) : serverIP(ip) {}
        ~NetConf() {};


        inline void SetRtmpLink(std::string ip) {serverIP = ip;}
        inline void SetServerIP(std::string ip) {rtmpLink = ip;}
        inline std::string GetRtmpLink() const {return rtmpLink;}
        inline std::string GetServerIP() const {return serverIP;}

        int ServerStart() const;
        int ServerStop() const;
        int ServerStatus() const;
        int RTMPconfig() const;
        static inline void exec(const char* cmd);

};