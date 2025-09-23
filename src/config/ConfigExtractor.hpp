#pragma once
#include <iostream>
#include <vector>
#include <unordered_set>
#include <regex>
#include <fstream>
#include <string>

#include "../server/Server.hpp"

class ConfigExtractor {

	private:

		void		extractPort(Server& serv, const std::string& line);
		void		extractAddress(Server& serv, const std::string& line);
		void		extractMaxBodySize(Server& serv, const std::string& line);
		static void	extractName(Server& serv, const std::string& line);
		static void	extractRoot(Server& serv, const std::string& line);
		static void	extractIndex(Server& serv, const std::string& line);
		static void	extractErrorPage(Server& serv, const std::string& line);

		static void	extractLocation(Location& loc, const std::string& line);
		static void	extractAllowedMethods(Location& loc, const std::string& line);
		static void	extractIndexLoc(Location& loc, const std::string& line);
		static void	extractAutoindex(Location& loc, const std::string& line);
		static void	extractCgiPath(Location& loc, const std::string& line);
		static void	extractCgiExt(Location& loc, const std::string& line);
		static void	extractUploadPath(Location& loc, const std::string& line);

		// Ilia added for redirect
		static void	extractReturn(Location& loc, const std::string& line);

	public:

		void		extractFields(std::vector<Server>& servs, std::ifstream& cfg);
		void		extractLocationFields(Server& serv, Location& loc, std::ifstream& cfg);
};
