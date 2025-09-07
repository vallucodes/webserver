NAME		= test_response
CXX			= c++
CXXFLAGS	= -std=c++17 -Wall -Wextra -Werror

SRC_DIR		= .
OBJ_DIR		= obj/

INCLUDES	= -I..

HEADERS		= ../Response.hpp

SRCS		= test_response.cpp \
			../Response.cpp

OBJS		= $(OBJ_DIR)/test_response.o \
			$(OBJ_DIR)/Response.o

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/test_response.o: test_response.cpp $(HEADERS)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/Response.o: ../Response.cpp
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
