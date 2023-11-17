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

int NetConf::RTMPconfig() const{

    std::ifstream file("/etc/nginx/nginx.conf"); 
    std::ofstream outfile("/etc/nginx/temp.conf");
    std::string toFind = "rtmp"; 
    std::string rtmpConfiguration = "\nrtmp {\n" 
	                                "    server {\n"
		                            "        listen "+port+";\n"
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
        return -1;
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
    return 0;
}

void NetConf::SearchBindPort(){

    const int startPort = 1024;
    for (int port = startPort; port < 65535; ++port) {
        if (NetConf::IsPortFree(port)) {
            this->port = std::to_string(port);
            break;
        }
    }

}

void NetConf::BindIp(){

    std::string ifconfigResult = NetConf::exec("ifconfig");

    std::regex ipRegex("(inet )(?:[0-9]{1,3}\\.){3}[0-9]{1,3}");

    std::smatch matches;
    std::regex_search(ifconfigResult, matches, ipRegex);
    std::vector<std::string> IPs;
    
    for(std::sregex_iterator i = std::sregex_iterator(ifconfigResult.begin(), ifconfigResult.end(), ipRegex); i != std::sregex_iterator();++i )
    {
        std::smatch m = *i;
        IPs.push_back(m.str());
    }


    for(std::string ip : IPs){
        ipRegex = "(?:[0-9]{1,3}\\.){3}[0-9]{1,3}";
        std::regex_search(ip, matches, ipRegex);
        if(matches[0].str() == "127.0.0.1"){
            continue;
        }

        this->externalIp = matches[0].str();
    }

    
}

void NetConf::BindRtmpLink(std::string key) {
    internalRtmpLink = "rtmp://"+internalIp+":"+port+"/live/"+key;
    externalRtmpLink = "rtmp://"+externalIp+":"+port+"/live/"+key;
}

void NetConf::BindRtmpLink() {
    internalRtmpLink = "rtmp://"+internalIp+":"+port+"/live";
    externalRtmpLink = "rtmp://"+externalIp+":"+port+"/live";
}