NAME		= test_router
CXX			= c++
CXXFLAGS	= -std=c++17 -g -Wall -Wextra -Werror

SRC_DIR		= .
OBJ_DIR		= obj/

INCLUDES	= -I.. -I../../../inc

HEADERS		= ../Router.hpp \
			../../../inc/Request.hpp \
			../../../inc/Response.hpp \
			../../../inc/AMessage.hpp

SRCS		= test_router_get.cpp \
			../Router.cpp \
			../../../src/message/AMessage.cpp \
			../../../src/request/Request.cpp \
			../../../src/response/Response.cpp

OBJS		= $(OBJ_DIR)/test_router_get.o \
			$(OBJ_DIR)/Router.o \
			$(OBJ_DIR)/AMessage.o \
			$(OBJ_DIR)/Request.o \
			$(OBJ_DIR)/Response.o

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/test_router_get.o: test_router_get.cpp $(HEADERS)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/Router.o: ../Router.cpp $(HEADERS)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/AMessage.o: ../../../src/message/AMessage.cpp ../../../inc/AMessage.hpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/Request.o: ../../../src/request/Request.cpp ../../../inc/Request.hpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/Response.o: ../../../src/response/Response.cpp ../../../inc/Response.hpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

run: $(NAME)
	./$(NAME)

.PHONY: all clean fclean re run
