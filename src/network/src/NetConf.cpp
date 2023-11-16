#include "NetConf.hpp"


void NetConf::ServerStart() const{
    try {
        NetConf::exec("sudo systemctl start nginx");
    } catch (const std::runtime_error& e) {
        std::string error = e.what();
        throw std::runtime_error(error+"\n");
    }

    std::cout << "Server started." << std::endl;
}

void NetConf::ServerStop() const{
    try {
        NetConf::exec("sudo systemctl stop nginx");
    } catch (const std::runtime_error& e) {
        std::string error = e.what();
        throw std::runtime_error(error+"\n");
    }

    std::cout << "Server stopped." << std::endl;
}

void NetConf::ServerStatus() const{
    try {
        NetConf::exec("sudo systemctl status nginx");
    } catch (const std::runtime_error& e) {
        std::string error = e.what();
        throw std::runtime_error(error+"\n");
    }
}

std::string NetConf::RTMPconfig() const{

    std::ifstream file("/etc/nginx/nginx.conf"); 
    std::ofstream outfile("/etc/nginx/temp.conf");
    std::string toFind = "rtmp"; 
    std::string rtmpConfiguration = "\nrtmp {\n" 
	                                "    server {\n"
		                            "        listen "+serverPort+";\n"
		                            "        chunk_size 4096;\n"
                                    "\n"
		                            "        application live {\n"
			                        "            live on;\n"
			                        "            record off;\n"
			                        "            allow publish all;\n"
			                        "            allow play all;\n"
		                            "        }\n"
	                                "    }\n"
                                    "}\n";
    
   
    if(!file.is_open()){
        return "diocan\n";
    }
       
    std::string line;
    while (getline(file, line)) {
        if (line.find(toFind) != std::string::npos) {
            break;
        }

        outfile << line  << std::endl;
    }

    outfile << rtmpConfiguration;
    file.close();
    outfile.close();
    std::remove("/etc/nginx/nginx.conf");
    std::rename("/etc/nginx/temp.conf", "/etc/nginx/nginx.conf");
    return "RTMP setup done.\n";
}

void NetConf::SearchBindPort(){

    const int startPort = 30000;
    for (int port = startPort; port < 65535; ++port) {
        if (NetConf::IsPortFree(port)) {
            this->serverPort = std::to_string(port);
            break;
        }
    }

}