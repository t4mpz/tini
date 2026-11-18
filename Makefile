CC_FLAGS= -O2 -Wall -Werror -Wpedantic
DEV_FLAGS= -fsanitize=address -g


debug:
	gcc  $(DEV_FLAGS) $(CC_FLAGS) src/tini.c main.c -o debug.e
