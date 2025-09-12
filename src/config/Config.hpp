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
		void	extractRoot(Server& serv, const std::string& line);
		void	extractIndex(Server& serv, const std::string& line);
		void	extractErrorPage(Server& serv, const std::string& line);

		void	extractLocation(Location& loc, const std::string& line);
		void	extractAllowedMethods(Location& loc, const std::string& line);
		void	extractIndexLoc(Location& loc, const std::string& line);
		void	extractAutoindex(Location& loc, const std::string& line);
		void	extractCgiPath(Location& loc, const std::string& line);
		void	extractCgiExt(Location& loc, const std::string& line);
		void	extractUploadPath(Location& loc, const std::string& line);

	public:

		std::vector<Server>	parse(const std::string& config);
};
