NAME		= test_router
CXX			= c++
CXXFLAGS	= -std=c++17 -Wall -Wextra -Werror

SRC_DIR		= .
OBJ_DIR		= obj/

INCLUDES	= -I..

HEADERS		= ../Router.hpp \
			../../response/Response.hpp

SRCS		= test_router_get.cpp \
			../Router.cpp \
			../../response/Response.cpp

OBJS		= $(OBJ_DIR)/test_router_get.o \
			$(OBJ_DIR)/Router.o \
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

$(OBJ_DIR)/Response.o: ../../response/Response.cpp ../../response/Response.hpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

run: $(NAME)
	./$(NAME)

.PHONY: all, clean, fclean, re, run
