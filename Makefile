HOSTTYPE ?=
ifeq ($(HOSTTYPE),)
	HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif
NAME = libft_malloc_$(HOSTTYPE).so

CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wno-unused-result\
	-D _MALLOC_DEBUG -D _MALLOC_DEBUG_SHOW_FREE -D _MALLOC_CONFLICT -g3

SRC_DIR = src/
SRC := $(shell find $(SRC_DIR) -type f -name "*.c")
HDR := $(shell find $(SRC_DIR) -type f -name "*.h")
DIRS := $(shell find $(SRC_DIR) -type d)
OBJ_DIR = obj/
OBJ := $(patsubst $(SRC_DIR)%.c,$(OBJ_DIR)%.o,$(SRC))
OBJ_DIRS := $(patsubst $(SRC_DIR)%,$(OBJ_DIR)%,$(DIRS))

UNIT_TESTS := $(shell find unit_test/ -type f -name "*.c")

all: $(NAME) tags

$(OBJ_DIRS):
	mkdir -p $(OBJ_DIRS)

$(NAME): $(OBJ_DIRS) $(OBJ)
	$(CC) -shared $(CFLAGS) -o $@ $(OBJ)

$(OBJ_DIR)%.o: $(SRC_DIR)%.c $(HDR)
	$(CC) $(CFLAGS) -o $@ -c $<

tags: $(SRC) $(HDR)
	ctags $(SRC) $(HDR)

clean:
	rm -r $(OBJ_DIR)

fclean: clean
	rm $(NAME)
	rm tags

re: fclean all

.PHONY: all clean fclean re
