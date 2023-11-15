#include "NetConf.hpp"


void NetConf::ServerStart() const{
    try {
        NetConf::exec("systemctl start nginx");
    } catch (const std::runtime_error& e) {
        std::string error = e.what();
        throw std::runtime_error(error+"\n");
    }

    std::cout << "Server started." << std::endl;
}

void NetConf::ServerStop() const{
    try {
        NetConf::exec("systemctl stop nginx");
    } catch (const std::runtime_error& e) {
        std::string error = e.what();
        throw std::runtime_error(error+"\n");
    }

    std::cout << "Server stopped." << std::endl;
}

void NetConf::ServerStatus() const{
    try {
        NetConf::exec("systemctl status nginx");
    } catch (const std::runtime_error& e) {
        std::string error = e.what();
        throw std::runtime_error(error+"\n");
    }
}

std::string NetConf::RTMPconfig() const{
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
    if (file.is_open()) {
        while (getline(file, line)) {

            if (line.find(toFind) != std::string::npos) {
                return "RTMP already setup.\n";
            }

        }
        file.close();


        std::ofstream file("/etc/nginx/nginx.conf");
        file << rtmpConfiguration << std::endl;
        return "RTMP setup done.\n";

    } else {
        return "Unable to open file.\n";
    }
}