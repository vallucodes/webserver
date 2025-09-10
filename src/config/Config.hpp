#pragma once
#include <iostream>
#include <vector>

#include "../server/Server.hpp"

class Config {

	private:

		void	extractPort(Server& serv, const std::string& line);
		void	extractAddress(Server& serv, const std::string& line);
		void	extractMaxBodySize(Server& serv, const std::string& line);
		void	extractName(Server& serv, const std::string& line);
	public:
		std::vector<Server>	parse(const std::string& config);
};
