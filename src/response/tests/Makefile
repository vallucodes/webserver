NAME		= test_response
CXX			= c++
CXXFLAGS	= -std=c++17 -Wall -Wextra -Werror

SRC_DIR		= src_test/
OBJ_DIR		= obj_test/

HEADERS		= ../Response.hpp

SRCS		= t_res_getters_setters.cpp \
			../Response.cpp

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

run: $(NAME)
	./$(NAME)

.PHONY: all, clean, fclean, re, run
