#include "ConfigExtractor.hpp"

void	ConfigExtractor::extractFields(std::vector<Server>& servs, std::ifstream& cfg) {
	std::string	line;
	Server		serv;

	while (std::getline(cfg, line)) {
		if (line.find("server {") != std::string::npos) {
			serv = Server();
			continue ;
		}
		// std::cout << line << std::endl;
		if (line.find("}") != std::string::npos) {
			// std::cout << "adding to servs" << std::endl;
			servs.push_back(serv);
			continue ;
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

void	ConfigExtractor::extractLocationFields(Server& serv, Location& loc, std::ifstream& cfg) {
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

		// Ilia added for redirect
		extractReturn(loc, line);
	}
}

void	ConfigExtractor::extractPort(Server& serv, const std::string& line) {
	std::regex	re("^\\s*listen\\s+(\\d+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		serv.setPort(std::stoi(match[1]));
	}
}

void	ConfigExtractor::extractAddress(Server& serv, const std::string& line) {
	std::regex	re("^\\s*host\\s+(\\d+\\.\\d+\\.\\d+\\.\\d+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		struct in_addr addr;
		inet_pton(AF_INET, match[1].str().c_str(), &addr);
		serv.setAddress(addr.s_addr);
	}
}

void	ConfigExtractor::extractMaxBodySize(Server& serv, const std::string& line) {
	std::regex	re("^\\s*client_max_body_size\\s+(\\d+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		// std::cout << "max_body_size found: " << match[1] << std::endl;
		serv.setMaxBodySize(std::stoi(match[1]));
	}
}

void	ConfigExtractor::extractName(Server& serv, const std::string& line) {
	std::regex	re("^\\s*server_name\\s+(\\S+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		// std::cout << "serv name found: " << match[1] << std::endl;
		serv.setName(match[1]);
	}
}

void	ConfigExtractor::extractRoot(Server& serv, const std::string& line) {
	std::regex	re("^\\s*root\\s+(\\S+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		// std::cout << "root found: " << match[1] << std::endl;
		serv.setRoot(match[1]);
	}
}

void	ConfigExtractor::extractIndex(Server& serv, const std::string& line) {
	std::regex	re("^\\s*index\\s+(\\S+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		// std::cout << "index found: " << match[1] << std::endl;
		serv.setIndex(match[1]);
	}
}

void	ConfigExtractor::extractErrorPage(Server& serv, const std::string& line) {
	std::regex	re("^\\s*error_page\\s+(\\S+)\\s+(\\S+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		int error_index = std::stoi(match[1]);
		// std::cout << "error pages found: " << match[1] << std::endl;
		// std::cout << "error pages found: " << match[2] << std::endl;
		serv.setErrorPage(error_index, match[2]);
	}
}

void	ConfigExtractor::extractLocation(Location& loc, const std::string& line) {
	std::regex	re("^\\s*location\\s+(\\S+)\\s*\\{$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		// std::cout << "location found: " << match[1] << std::endl;
		loc.location = match[1];
	}
}

void	ConfigExtractor::extractAllowedMethods(Location& loc, const std::string& line) {
	std::regex	re("^\\s*allow_methods\\s+(.+)$");
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

void	ConfigExtractor::extractIndexLoc(Location& loc, const std::string& line) {
	std::regex	re("^\\s*index\\s+(\\S+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		// std::cout << "index found: " << match[1] << std::endl;
		loc.index = match[1];
	}
}

void	ConfigExtractor::extractAutoindex(Location& loc, const std::string& line) {
	std::regex	re("^\\s*autoindex\\s+(\\S+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		// std::cout << "autoindex found: " << match[1] << std::endl;
		if (match[1] == "on")
			loc.autoindex = true;
		else if (match[1] == "off")
			loc.autoindex = false;
	}
}

void	ConfigExtractor::extractCgiPath(Location& loc, const std::string& line) {
	std::regex	re("^\\s*cgi_path\\s+(\\S+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		// std::cout << "cgi_path found: " << match[1] << std::endl;
		loc.cgi_path = match[1];
	}
}

void	ConfigExtractor::extractCgiExt(Location& loc, const std::string& line) {
	std::regex	re("^\\s*cgi_ext\\s+(.+)$");
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

void	ConfigExtractor::extractUploadPath(Location& loc, const std::string& line) {
	std::regex	re("^\\s*upload_to\\s+(\\S+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		// std::cout << "cgi_path found: " << match[1] << std::endl;
		loc.upload_path = match[1];
	}
}

// Ilia added for redirect
void	ConfigExtractor::extractReturn(Location& loc, const std::string& line) {
	std::regex	re("^\\s*return\\s+(\\S+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		// std::cout << "return_url found: " << match[1] << std::endl;
		loc.return_url = match[1];
	}
}
