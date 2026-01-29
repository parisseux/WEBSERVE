NAME = webserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iinclude -g
RM = rm -f

SRC_DIR = src
OBJ_DIR = obj

PINK = \033[1;95m
GREEN = \033[1;32m
RESET = \033[0m

SRCS = $(shell find $(SRC_DIR) -type f -name "*.cpp")

OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "$(PINK)Compilation of $(NAME) successful ✔$(RESET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@$(RM) -r $(OBJ_DIR)
	@echo "$(GREEN)Cleaning successful for $(NAME) ✔$(RESET)"

fclean: clean
	@$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
