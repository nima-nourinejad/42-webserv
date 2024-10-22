# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/10/22 16:45:01 by asohrabi          #+#    #+#              #
#    Updated: 2024/10/22 18:02:10 by asohrabi         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Compiler and flags
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++11 -Iincludes

# Source and object files
SRCS = srcs/CGIHandler.cpp srcs/HttpHandler.cpp srcs/main.cpp srcs/Request.cpp \
		srcs/SystemCallError.cpp

OBJS = $(SRCS:.cpp=.o)

# Target executable
TARGET = webserv

# Rules
all: $(TARGET)

$(TARGET): $(OBJS)
	@$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)

fclean: clean
	@rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re
