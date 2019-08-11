##
## Makefile for bsq in /home/guinau_v/Prog_elem_C/BSQ
## 
## Made by Vincent Guinaudeau
## Login   <guinau_v@epitech.net>
## 
## Started on  Mon Jan 12 18:20:47 2015 Vincent Guinaudeau
## Last update Sun Jan 18 22:59:28 2015 Vincent Guinaudeau
##

NAME	= buddhabrot

SRC		= src/main.c \
          src/parser.c \
          src/fract.c \
          src/view.c \
          src/list.c \
          src/random.c \
          src/tree.c \
          src/scan.c \
          src/tree_stat.c

OBJ		= $(SRC:.c=.o)

CC		= gcc

CFLAGS	= -Iinclude/ -Wall -Wextra -O3
# CFLAGS	= -Iinclude/ -Wall -Wextra -O0 -g3 -pg

LDFLAGS = -lpthread -lm

RM		= rm -f

MAKE	= make --no-print-directory

$(NAME):	$(OBJ)
	$(CC) $(OBJ) -o $(NAME) $(CFLAGS) $(LDFLAGS)

all:	$(NAME)

clean:
	$(RM) $(OBJ)

fclean:	clean
	$(RM) $(NAME)

re:	fclean all

.PHONY: all clean fclean re debug release
