# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/10/22 16:45:01 by asohrabi          #+#    #+#              #
#    Updated: 2024/11/13 15:56:57 by asohrabi         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Compiler and flags
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++11 -Iincludes

# Source and object files
SRCS = $(wildcard *.cpp)

SRCS_DIR = srcs
OBJs_DIR = objs
OBJS = $(SRCS:$(SRCS_DIR)/%.cpp=$(OBJs_DIR)/%.o)

# Target executable
TARGET = webserv

# Rules
all: $(TARGET)

$(TARGET): $(OBJS)
	@$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

$(OBJs_DIR)/%.o: $(SRCS_DIR)/%.cpp | $(OBJs_DIR)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJs_DIR):
	@mkdir -p $(OBJs_DIR)

clean:
	@rm -rf $(OBJs_DIR)

fclean: clean
	@rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re
