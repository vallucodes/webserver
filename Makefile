NAME		= webserv
CXX			= c++
CXXFLAGS	= -g -std=c++20 -Wall -Wextra -Werror

SRC_DIR		= src/
OBJ_DIR		= obj/

INCLUDES	= -I ./inc
HEADERS		= inc/webserv.hpp \
				src/config/Config.hpp \
				src/config/ConfigExtractor.hpp \
				src/config/ConfigValidator.hpp \
				src/server/devHelpers.hpp \
				src/server/HelperFunctions.hpp \
				src/server/Cluster.hpp \
				src/server/Server.hpp \
				src/router/Router.hpp \
				src/router/HttpConstants.hpp \
				src/router/RequestProcessor.hpp \
				src/router/handlers/Handlers.hpp \
				src/request/Request.hpp \
				src/response/Response.hpp \
				src/message/AMessage.hpp \
				src/parser/Parser.hpp

SRCS		= src/main.cpp \
				src/config/Config.cpp \
				src/config/ConfigExtractor.cpp \
				src/config/ConfigValidator.cpp \
				src/server/devHelpers.cpp \
				src/server/HelperFunctions.cpp \
				src/server/Cluster.cpp \
				src/server/Server.cpp \
				src/router/Router.cpp \
				src/router/RequestProcessor.cpp \
				src/router/handlers/Handlers.cpp \
				src/request/Request.cpp \
				src/response/Response.cpp \
				src/message/AMessage.cpp \
				src/parser/Parser.cpp \
				src/parser/ParserUtils.cpp

OBJS		= $(patsubst $(SRC_DIR)%.cpp,$(OBJ_DIR)%.o,$(SRCS))

all: $(NAME)

$(NAME): $(OBJS) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(INCLUDES) -o $(NAME)

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all, clean, fclean, re
