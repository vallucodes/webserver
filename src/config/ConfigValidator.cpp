#include "ConfigValidator.hpp"

void	ConfigValidator::validateFields(std::ifstream& cfg) {
	std::stack<std::string>	blockstack;
	std::set<std::string>	locations;

	std::regex	close_block("^\\s*\\}$");

	LocationType	current_type = NONE;
	bool			location_present = false;
	std::string		line;

	std::smatch		match;

	while (std::getline(cfg, line)) {
		size_t first_non_space = line.find_first_not_of(" \t");
		if (first_non_space == std::string::npos || line[first_non_space] == '#')
			continue ;

		if (line.ends_with("{")) {
			handleOpenBlock(blockstack, line, current_type, location_present, locations);
			continue ;
		}

		if (std::regex_match(line, match, close_block)) {
			handleCloseBlock(blockstack, line, current_type, location_present);
			continue ;
		}

		handleKeyword(blockstack, line);
	}
	if (!blockstack.empty())
		throw std::runtime_error("Error: Config: Missing closing curly brace (syntax error)");
}

void	ConfigValidator::handleOpenBlock(std::stack<std::string>& blockstack, const std::string& line,
				LocationType& current_type, bool& location_present, std::set<std::string>& locations) {
	std::regex	server("^\\s*server\\s*\\{$");
	std::regex	location("^\\s*location\\s+(\\S+)\\s+\\{$");

	std::smatch				match;

	std::string				blocktype;

	if (std::regex_match(line, match, server)) {
		blocktype = "server";
		locations.clear();
		resetDirectivesFlags(blocktype);
	}
	else if (std::regex_match(line, match, location)) {
		if (blockstack.top() == "location")
			throw std::runtime_error("Error: Config: Nested 'location' block is not allowed: " + line);
		blocktype = "location";
		if (!locations.insert(match[1]).second)
			throw std::runtime_error("Error: Config: Duplicate location: " + line);
		if (!validateLocation(line, current_type, location_present))
			throw std::runtime_error("Error: Config: Invalid value for directive: location");
		resetDirectivesFlags(blocktype);
	}
	else
		throw std::runtime_error("Error: Config: Invalid block type: " + line);

	if (!blockstack.empty() && blocktype == "server")
		throw std::runtime_error("Error: Config: 'server' block must be top-level only: " + line);

	blockstack.push(blocktype);
}

void	ConfigValidator::handleCloseBlock(std::stack<std::string>& blockstack, const std::string& line, LocationType current_type, bool location_present) {
	if (blockstack.empty())
		throw std::runtime_error("Error: Config: Unbalanced }: " + line);
	verifyMandatoryDirectives(blockstack.top(), current_type);
	if (blockstack.top() == "server" && !location_present)
		throw std::runtime_error("Error: Config: Missing directory type of location");
	blockstack.pop();
}

void	ConfigValidator::handleKeyword(std::stack<std::string>&	blockstack, const std::string& line) {
	if (blockstack.empty())
		throw std::runtime_error("Error: Config: Keyword outside of any block: " + line);
	std::string currentBlock = blockstack.top();
	if (currentBlock == "server")
		validateKeyword(line, "server");
	if (currentBlock == "location")
		validateKeyword(line, "location");
}

ConfigValidator::ConfigValidator() {
	_mandatory_server_directives = {
		"listen",
		"server_name",
		"host",
		"root"
	};

	_mandatory_location_directives_directory = {
		"allow_methods",
		"index"
	};

	_mandatory_location_directives_cgi = {
		"allow_methods",
		"cgi_path",
		"cgi_ext"
	};

	_server_directives = {
		{"listen", std::regex("^\\s*listen\\s+\\d+$"), validatePort},
		{"server_name", std::regex("^\\s*server_name\\s+\\S+$"), nullptr},
		{"host", std::regex("^\\s*host\\s+\\d+\\.\\d+\\.\\d+\\.\\d+$"), validateIP},
		{"root", std::regex("^\\s*root\\s+\\S+$"), nullptr},
		{"index", std::regex("^\\s*index\\s+\\S+$"), validateIndex},
		{"client_max_body_size", std::regex("^\\s*client_max_body_size\\s+\\d+$"), validateMaxBodySize},
		{"error_page", std::regex("^\\s*error_page\\s+\\d+\\s+\\S+$"), validateErrorPage}
	};

	_location_directives = {
		{"location", std::regex(""), nullptr},
		{"allow_methods", std::regex("^\\s*allow_methods(\\s+\\S+){1,9}$"), validateMethods},
		{"index", std::regex("^\\s*index\\s+\\S+$"), validateIndex},
		{"autoindex", std::regex("^\\s*autoindex\\s+\\S+$"), validateAutoindex},
		{"cgi_path", std::regex("^\\s*cgi_path\\s+\\S+$"), nullptr},
		{"cgi_ext", std::regex("^\\s*cgi_ext(\\s+\\S+)+$"), validateExt},
		{"upload_to", std::regex("^\\s*upload_to\\s+\\S+$"), nullptr},
		{"return", std::regex("^\\s*return\\s+\\S+$"), nullptr}
	};
}

std::vector<std::string> ConfigValidator:: _methods = {
	"GET",
	"POST",
	"DELETE",
	"HEAD",
	"PUT",
	"PATCH",
	"OPTIONS",
	"CONNECT",
	"TRACE"
};

std::vector<std::string> ConfigValidator::_cgi_extensions = {
	".py",
	".php"
};

bool	ConfigValidator::validatePort(const std::string& line) {
	size_t pos = line.find_last_of(' ');
	if (pos == std::string::npos)
		return false;

	int port = std::stoi(line.substr(pos + 1));
	if (port >= 1024 && port <= 49151) {

		std::vector<int> restricted_ports =
		{1025, 1080, 1098, 1099,
		1433, 1521, 1723, 3306,
		3389, 5432, 5900};
		for (size_t i = 6000; i < 6064; ++i)
			restricted_ports.push_back(i);

		auto it = std::find(restricted_ports.begin(), restricted_ports.end(), port);
		if (it != restricted_ports.end())
			return false;
		return true;
	}
	return false;
}

bool	ConfigValidator::validateIP(const std::string& line) {
	std::regex	re("host\\s+(\\d+\\.\\d+\\.\\d+\\.\\d+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re))
	{
		struct in_addr addr;
		if (!inet_pton(AF_INET, match[1].str().c_str(), &addr))
			return false;
	}
	return true;
}

bool	ConfigValidator::validateIndex(const std::string& line) {
	std::regex	re(".html$");
	if (!std::regex_search(line, re))
		return false;
	return true;
}

bool	ConfigValidator::validateMaxBodySize(const std::string& line) {
	size_t pos = line.find_last_of(' ');
	if (pos == std::string::npos)
		return false;

	int body_size = std::stoi(line.substr(pos + 1));
	if (body_size >= 0 && body_size <= MAX_BODY_SIZE)
		return true;
	return false;
}

bool	ConfigValidator::validateErrorPage(const std::string& line) {
	std::regex	re1("^\\s*error_page\\s+(\\d+)\\s+(\\S+)$");
	std::smatch	match;
	if (!std::regex_search(line, match, re1))
		return false;
	int error_nb = std::stoi(match[1]);
	int error_nb_file = std::stoi(match[2]);
	if (error_nb != error_nb_file)
		return false;
	std::regex	re2(".html$");
	if (!std::regex_search(line, re2))
		return false;
	return true;
}

bool	ConfigValidator::validateLocation(const std::string& line, LocationType& type, bool& location_present) {
	std::regex	re("^\\s*location\\s+(\\S+)\\s+\\{$");
	std::smatch	match;
	if (std::regex_search(line, match, re)) {
		if (match[1].str()[0] != '/' && match[1].str()[0] != '.')
			return false;
		for (auto& d : _location_directives) {
			if (d.name == "location" && match[1].str()[0] == '/') {
				// std::cout << "Curr match: " << match[1] << ", setting type DIRECOTRY\n";
				d.location_type = DIRECTORY;
				type = d.location_type;
				location_present = true;
			}
			else if (d.name == "location" && match[1].str()[0] == '.') {
				// std::cout << "Curr match: " << match[1] << ", setting type FILE_EXTENSION\n";
				d.location_type = FILE_EXTENSION;
				type = d.location_type;
			}
		}
		return true;
	}
	return false;
}

bool	ConfigValidator::validateMethods(const std::string& line) {
	std::regex	re("^\\s*allow_methods(\\s+\\S+){1,9}$");
	if (std::regex_match(line, re)) {
		size_t pos = line.find("allow_methods");
		std::string methods_str = line.substr(pos + 13);

		std::istringstream iss(methods_str);
		std::string method;
		while (iss >> method) {
			auto it = std::find(_methods.begin(), _methods.end(), method);
			if (it == _methods.end())
				return false;
		}
		return true;
	}
	return false;
}

bool	ConfigValidator::validateExt(const std::string& line) {
	std::regex	re("^\\s*cgi_ext(\\s+\\S+)+$");
	if (std::regex_match(line, re)) {
		size_t pos = line.find("cgi_ext");
		std::string methods_str = line.substr(pos + 7);

		std::istringstream iss(methods_str);
		std::string method;
		while (iss >> method) {
			auto it = std::find(_cgi_extensions.begin(), _cgi_extensions.end(), method);
			if (it == _cgi_extensions.end())
				return false;
		}
		return true;
	}
	return false;
}

bool	ConfigValidator::validateAutoindex(const std::string& line) {
	std::regex	re("^\\s*autoindex\\s+(\\S+)$");
	std::smatch	match;
	if (std::regex_search(line, match, re)) {
		if (match[1] != "on" && match[1] != "off")
			return false;
		return true;
	}
	return false;
}

void	ConfigValidator::validateKeyword(const std::string& line, const std::string& context) {
	bool match = false;
	std::vector<Directive>& directives =
		(context == "server") ? _server_directives : _location_directives;
	for (auto &d : directives) {
		if (std::regex_match(line, d.pattern)) {
			match = true;
			if (d.isSet)
				throw std::runtime_error("Error: Config: Repeated directive: " + line);
			if (d.valueChecker && !d.valueChecker(line))
				throw std::runtime_error("Error: Config: Invalid value for directive: " + d.name);
			d.isSet = true;
		}
	}
	if (!match)
		throw std::runtime_error("Error: Config: Malformed directive: " + line);
}

void	ConfigValidator::resetDirectivesFlags(const std::string& blocktype) {
	if (blocktype == "server") {
		for (auto &d : _server_directives)
			d.isSet = false;
	}
	if (blocktype == "location") {
		for (auto &d : _location_directives)
			d.isSet = false;
	}
}

void	ConfigValidator::verifyMandatoryDirectives(const std::string& blocktype, LocationType loctype) {
	if (blocktype == "server") {
		for (auto& d : _server_directives) {
			// std::cout << "Checking: " << d.name << ", value: " << d.isSet << std::endl;
			if (_mandatory_server_directives.count(d.name) && !d.isSet)
				throw std::runtime_error("Error: Config: Missing mandatory server directory: " + d.name);
		}
	}
	else if (blocktype == "location") {
		const auto& mandatory_list =
		(loctype == DIRECTORY) ? _mandatory_location_directives_directory
								: _mandatory_location_directives_cgi;
		for (auto& d : _location_directives) {
				// std::cout << "Checking: " << d.name << ", type: " << d.location_type << std::endl;
			if (mandatory_list.count(d.name) && !d.isSet)
				throw std::runtime_error("Error: Config: Missing mandatory location directory: " + d.name);
		}
	}
	else
		throw std::runtime_error("Unreachable: verifyMandatoryDirectives()");
}
