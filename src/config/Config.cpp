#include "Config.hpp"

void	Config::validate(const std::string& config) {
	std::ifstream cfg(config);
	if (!cfg.is_open())
		throw std::runtime_error("Error: could not open config file.");

	validator.validateFields(cfg);
}

std::vector<Server>	Config::parse(const std::string& config) {
	// Server serv;
	std::vector<Server> servs;

	// std::cout << config << std::endl;
	std::ifstream cfg(config);
	if (!cfg.is_open())
		throw std::runtime_error("Error: could not open config file.");

	parser.extractFields(servs, cfg);
	return servs;
}
