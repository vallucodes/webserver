#include <regex>
#include <fstream>
#include <string>

#include "Config.hpp"

std::vector<Server>	Config::parse(const std::string& config) {
	Server serv;
	std::vector<Server> servs;

	std::cout << config << std::endl;
	std::ifstream cfg(config);
	if (!cfg.is_open()) {
		throw std::runtime_error("Error: could not open config file.");
	}

	bool inServer = false;
	bool inLocation = false;

	std::string line;
	while (std::getline(cfg, line)) {
		// std::cout << line << std::endl;
		if (line.find("server {") != std::string::npos){
			serv = Server();
			inServer = true;
			continue ;
		}

		if (line.find("location ") != std::string::npos) { // change this to regex to catch wrong "{
			inLocation = true;
			continue ;
		}

		if (line.find("}") != std::string::npos && inLocation) {
			inLocation = false;
			continue ;
		}

		if (line.find("}") != std::string::npos && inServer && !inLocation) {
			servs.push_back(serv);
			inServer = false;
			continue ;
		}

		if (!inServer)
			continue ;

		extractPort(serv, line);
		extractAddress(serv, line);
		extractMaxBodySize(serv, line);
		extractName(serv, line);
	}

	return servs;
}

void	Config::extractPort(Server& serv, const std::string& line) {
	std::regex	re("listen\\s+(\\d+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		// std::cout << "port found: " << match[1] << std::endl;
		serv.setPort(std::stoi(match[1]));
	}
}

void	Config::extractAddress(Server& serv, const std::string& line) {
	std::regex	re("host\\s+(\\d+\\.\\d+\\.\\d+\\.\\d+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		struct in_addr addr;
		if (inet_pton(AF_INET, match[1].str().c_str(), &addr) == 1){
			// std::cout << "address found: " << match[1] << std::endl;
			serv.setAddress(addr.s_addr);
		}
		else
			throw std::runtime_error("Error: Invalid IP address");
	}
}

void	Config::extractMaxBodySize(Server& serv, const std::string& line) {
	std::regex	re("client_max_body_size\\s+(\\d+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		// std::cout << "max_body_size found: " << match[1] << std::endl;
		serv.setMaxBodySize(std::stoi(match[1]));
	}
}

void	Config::extractName(Server& serv, const std::string& line) {
	std::regex	re("server_name\\s+(\\S+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		std::cout << "name found: " << match[1] << std::endl;
		serv.setName(match[1]);
	}
}
