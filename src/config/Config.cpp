#include <regex>
#include <fstream>
#include <string>

#include "Config.hpp"

void	Config::checkServerKeywords(const std::string& line) {

	const std::vector<std::regex> directives = {
		std::regex("^\\s*port\\s+\\d+$"),
		std::regex("^\\s*server_name\\s+\\S+$"),
		std::regex("^\\s*host\\s+\\(\\d{1,3}\\.){3}\\d{1,3}$"),
		std::regex("^\\s*root\\s+\\S+$"),
		std::regex("^\\s*index\\s+\\S+$"),
		std::regex("^\\s*client_max_body_size\\s+\\d+$"),
		std::regex("^\\s*error_page\\s+\\d+\\s+\\S+$")
	};

	bool match = false;

	for (auto &r : directives) {
		if (std::regex_match(line, r)) {
			match = true;
			return ;
		}
	}
	if (!match)
		throw std::runtime_error("Error: Config: Malformed directive: " + line);
}

void	Config::checkLocationKeywords(const std::string& line) {

}

std::vector<Server>	Config::validate(const std::string& config) {

	// std::cout << config << std::endl;
	std::ifstream cfg(config);
	if (!cfg.is_open())
		throw std::runtime_error("Error: could not open config file.");

	std::stack<std::string> blockstack;

	std::regex	server("^\\s*server\\s*\\{$");
	std::regex	location("^\\s*location\\s+[^\\s]+\\s*{$");
	std::regex	close_block("^\\s*\\}$");

	std::string line;
	std::string blocktype;
	std::smatch match;
	while (std::getline(cfg, line)) {
		if (line.empty() || line[0] == '#')
			continue ;
		if (line.ends_with("{")) {
			if (std::regex_match(line, match, server))
				blocktype = "server";
			else if (std::regex_match(line, match, location))
				blocktype = "location";
			else
				throw std::runtime_error("Error: Config: Invalid block type: " + line);
			blockstack.push(blocktype);
			continue ;
		}
		if (std::regex_match(line, match, close_block)) {
			if (blockstack.empty())
				throw std::runtime_error("Error: Config: Unbalanced }: " + line);
			blockstack.pop();
			continue;
		}
		if (blockstack.empty())
			throw std::runtime_error("Error: Config: Keyword outside of any block: " + line);

		std::string currentBlock = blockstack.top();

		if (currentBlock == "server")
			checkServerKeywords(line);
		if (currentBlock == "location")
			checkLocationKeywords(line);
	}
}

void	Config::extractLocationFields(Server& serv, Location& loc, std::ifstream& cfg) {
	std::string line;

	while (std::getline(cfg, line)) {
		// std::cout << line << std::endl;
		if (line.find("}") != std::string::npos) {
			serv.setLocation(loc);
			break ;
		}
		extractAllowedMethods(loc, line);
		extractIndexLoc(loc, line);
		extractAutoindex(loc, line);
		extractCgiPath(loc, line);
		extractCgiExt(loc, line);
		extractUploadPath(loc, line);
	}
}

void	Config::extractServerFields(std::vector<Server>& servs, std::ifstream& cfg) {
	std::string line;
	Server serv;

	while (std::getline(cfg, line)) {
		// std::cout << line << std::endl;
		if (line.find("}") != std::string::npos) {
			servs.push_back(serv);
			break ;
		}
		extractPort(serv, line);
		extractAddress(serv, line);
		extractMaxBodySize(serv, line);
		extractName(serv, line);
		extractRoot(serv, line);
		extractIndex(serv, line);
		extractErrorPage(serv, line);
		if (line.find("location ") != std::string::npos) {
			Location loc;
			extractLocation(loc, line);
			extractLocationFields(serv, loc, cfg);
		}
	}
}

std::vector<Server>	Config::parse(const std::string& config) {
	Server serv;
	std::vector<Server> servs;

	// std::cout << config << std::endl;
	std::ifstream cfg(config);
	if (!cfg.is_open())
		throw std::runtime_error("Error: could not open config file.");

	std::string line;
	while (std::getline(cfg, line)) {
		// std::cout << line << std::endl;
		if (line.find("server {") != std::string::npos)
			extractServerFields(servs, cfg);
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
		// std::cout << "serv name found: " << match[1] << std::endl;
		serv.setName(match[1]);
	}
}

void	Config::extractRoot(Server& serv, const std::string& line) {
	std::regex	re("root\\s+(\\S+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		// std::cout << "root found: " << match[1] << std::endl;
		serv.setRoot(match[1]);
	}
}

void	Config::extractIndex(Server& serv, const std::string& line) {
	std::regex	re("index\\s+(\\S+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		// std::cout << "index found: " << match[1] << std::endl;
		serv.setIndex(match[1]);
	}
}

void	Config::extractErrorPage(Server& serv, const std::string& line) {
	std::regex	re("error_page\\s+(\\S+)\\s+(\\S+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		int error_index = std::stoi(match[1]);
		// std::cout << "error pages found: " << match[1] << std::endl;
		// std::cout << "error pages found: " << match[2] << std::endl;
		serv.setErrorPage(error_index, match[2]);
	}
}

void	Config::extractLocation(Location& loc, const std::string& line) {
	std::regex	re("location\\s+(\\S+)\\s*\\{$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		// std::cout << "location found: " << match[1] << std::endl;
		loc.location = match[1];
	}
}

void	Config::extractAllowedMethods(Location& loc, const std::string& line) {
	std::regex	re("allow_methods\\s+(.+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		std::istringstream iss(match[1]);
		std::string token;
		std::vector<std::string> methods;
		while (iss >> token)
			methods.push_back(token);
		// std::cout << "allowed mehods found: " << match[1] << std::endl;
		loc.allowed_methods = methods;
	}
}

void	Config::extractIndexLoc(Location& loc, const std::string& line) {
	std::regex	re("index\\s+(\\S+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		// std::cout << "index found: " << match[1] << std::endl;
		loc.index = match[1];
	}
}

void	Config::extractAutoindex(Location& loc, const std::string& line) {
	std::regex	re("autoindex\\s+(\\S+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		// std::cout << "autoindex found: " << match[1] << std::endl;
		if (match[1] == "on")
			loc.autoindex = true;
		else if (match[1] == "off")
			loc.autoindex = false;
		else
			throw std::runtime_error("Wrong formatting of autoindex");
	}
}

void	Config::extractCgiPath(Location& loc, const std::string& line) {
	std::regex	re("cgi_path\\s+(\\S+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		// std::cout << "cgi_path found: " << match[1] << std::endl;
		loc.cgi_path = match[1];
	}
}

void	Config::extractCgiExt(Location& loc, const std::string& line) {
	std::regex	re("cgi_ext\\s+(.+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		std::istringstream iss(match[1]);
		std::string token;
		std::vector<std::string> types;
		while (iss >> token)
			types.push_back(token);
		// std::cout << "allowed mehods found: " << match[1] << std::endl;
		loc.cgi_ext = types;
	}
}

void	Config::extractUploadPath(Location& loc, const std::string& line) {
	std::regex	re("upload_to\\s+(\\S+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		// std::cout << "cgi_path found: " << match[1] << std::endl;
		loc.upload_path = match[1];
	}
}
