CXX			= c++
CXXFLAGS	= -std=c++17 -g -Wall -Wextra -Werror

SRC_DIR		= .
OBJ_DIR		= obj/

INCLUDES	= -I.. -I../.. -I../../..

HEADERS		= ../Router.hpp \
			../handlers/Handlers.hpp \
			../../request/Request.hpp \
			../../response/Response.hpp \
			../../message/AMessage.hpp

# Utility file for tests
UTILS		= utils.cpp

# Common object files
COMMON_OBJS	= $(OBJ_DIR)/Router.o \
			$(OBJ_DIR)/Handlers.o \
			$(OBJ_DIR)/AMessage.o \
			$(OBJ_DIR)/Request.o \
			$(OBJ_DIR)/Response.o \
			$(OBJ_DIR)/utils.o

# Individual test executables
TEST_ROUTER_GET		= test_router_get
TEST_ADD_ROUTE		= test_addRoute

all: $(TEST_ROUTER_GET) $(TEST_ADD_ROUTE)

$(TEST_ROUTER_GET): $(OBJ_DIR)/test_router_get.o $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TEST_ADD_ROUTE): $(OBJ_DIR)/test_addRoute.o $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ_DIR)/test_router_get.o: test_router_get.cpp $(HEADERS)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/test_addRoute.o: test_addRoute.cpp $(HEADERS)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/Router.o: ../Router.cpp $(HEADERS)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/Handlers.o: ../handlers/Handlers.cpp $(HEADERS)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/AMessage.o: ../../message/AMessage.cpp ../../message/AMessage.hpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/Request.o: ../../request/Request.cpp ../../request/Request.hpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/Response.o: ../../response/Response.cpp ../../response/Response.hpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/utils.o: $(UTILS)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(TEST_ROUTER_GET) $(TEST_ADD_ROUTE)

re: fclean all

run: $(TEST_ROUTER_GET)
	./$(TEST_ROUTER_GET)

run_addroute: $(TEST_ADD_ROUTE)
	./$(TEST_ADD_ROUTE)

.PHONY: all clean fclean re run run_addroute
