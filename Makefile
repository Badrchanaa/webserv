# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: bchanaa <marvin@42.fr>                     +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/04/21 12:11:54 by bchanaa           #+#    #+#              #
#    Updated: 2024/07/20 09:42:00 by bchanaa          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #
# COLORS 
NOCOL=\033[0m
#RED=\033[1;31m
YEL=\033[1;33m
GRN=\033[1;32m
BLU=\033[1;34m


LYELLOW = \033[38;5;222m
BEIGE = \033[38;5;223m
LGREEN = \033[38;5;155m

BOLD = \033[1m
CUR = \033[3m
UL = \033[4m
UP = \033[A

# NAMES
NAME = webserv 

# MAKE
MAKE = make

# COMPILER
CC = c++

# PATHS
OBJ_PATH = .bin
SRC_PATH = ./src


# LIBS

CFLAGS = -Wall -Wextra -Werror -std=c++98 -I includes

# ANIMATION
SRCS_COUNT = 0
SRCS_TOT = ${shell find $(SRC_PATH)/ -type f -name '*.cpp' | wc -l}
SRCS_PRCT = ${shell expr 100 \* ${SRCS_COUNT} / ${SRCS_TOT}}
#determine the length of the progress bar.
BAR =  ${shell expr 23 \* ${SRCS_COUNT} / ${SRCS_TOT}}


ifdef DEBUG
	CFLAGS += -fsanitize=address -g3
endif

# SOURCES
CONFIG_PARSING_SOURCES = main.cpp ConfigFile.cpp  Methodes_Validates.cpp Utils.cpp

HTTP_TEST_SOURCES = $(SRC_PATH)/http/test.cpp
HTTP_SOURCES = $(filter-out $(HTTP_TEST_SOURCES), $(wildcard $(SRC_PATH)/http/*.cpp))

ALL_SOURCES = $(CONFIG_PARSING_SOURCES)
vpath %.cpp $(SRC_PATH)/Parsing 
vpath %.cpp $(SRC_PATH)/http
vpath %.hpp includes/
# OBJ_FILES = $(ALL_SOURCES:%.c=%.o)

HTTP_OBJ_FILES = $(addprefix $(OBJ_PATH)/, $(HTTP_SOURCES:$(SRC_PATH)/http/%.cpp=%.o) $(HTTP_TEST_SOURCES:$(SRC_PATH)/http/%.cpp=%.o))
OBJ_PARSING_FILES = $(CONFIG_PARSING_SOURCES:%.cpp=%.o)

# OBJ_PIPX_FILES = $(PIPX_SOURCES:%.c=%.o)
OBJ_PARSING_FILES = $(CONFIG_PARSING_SOURCES:%.cpp=%.o)
OBJ_FILES = $(addprefix $(OBJ_PATH)/, $(OBJ_PARSING_FILES))

all: $(NAME)
$(NAME): $(OBJ_FILES)
	@echo "\n"
# @echo "i am here\n"
	@$(CC) $^ -o $@ $(CFLAGS) #-fsanitize=address
# @echo "i am here\n"
	@echo "\n\n\n   ${BOLD}${CUR}${LYELLOW}WEBSERV COMPILED ✨${DEF}${NOCOL}\n"

http_test: $(HTTP_OBJ_FILES)
	$(CC) -o $@ $(CFLAGS) $^


# #is used to redirect both standard output (stdout) and standard error (stderr) to /dev/null

$(OBJ_PATH):
	@mkdir -p $(OBJ_PATH) > /dev/null 2>&1

$(OBJ_PATH)/%.o: %.cpp | $(OBJ_PATH)
#The eval command allows you to perform dynamic evaluation and assignment within the Makefile.
	@${eval SRCS_COUNT = ${shell expr ${SRCS_COUNT} + 1}}
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo ""
	@echo " ${BOLD}${CUR}${BEIGE}-> Compiling ${DEF}${BOLD}${LYELLOW}[WEBSERV]${DEF}"
	@printf " ${BEIGE}   [${LGREEN}%-23.${BAR}s${BEIGE}] [%d/%d (%d%%)]${DEF}" "***********************" ${SRCS_COUNT} ${SRCS_TOT} ${SRCS_PRCT}
#@echo "${UP}${UP}${UP}": Uses ANSI escape sequences (${UP}) to move the cursor up three lines
	@echo "${UP}${UP}${UP}"

debug:
	@echo "\n   ${BOLD}${UL}${MAGENTA}DEBUGGING MODE${DEF}"
	@${MAKE} DEBUG=1

clean:
	@rm -rf $(OBJ_PATH)

fclean: clean
	@rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re
