##
## Makefile for bsq in /home/guinau_v/Prog_elem_C/BSQ
## 
## Made by Vincent Guinaudeau
## Login   <guinau_v@epitech.net>
## 
## Started on  Mon Jan 12 18:20:47 2015 Vincent Guinaudeau
## Last update Sun Jan 18 22:59:28 2015 Vincent Guinaudeau
##

NAME	= buddabrot

SRC		= main.c \
		  util.c \
		  io.c \

OBJ		= $(SRC:.c=.o)

CC		= clang

CFLAGS	= -Wall -Wextra

RM		= rm -f

MAKE	= make --no-print-directory

$(NAME):	$(OBJ)
	$(CC) $(OBJ) -o $(NAME) $(CFLAGS)

all:	$(NAME)

clean:
	$(RM) $(OBJ)

fclean:	clean
	$(RM) $(NAME)

re:	fclean all

.PHONY: all clean fclean re debug release