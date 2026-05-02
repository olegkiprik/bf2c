COMMON_FLAGS = -pedantic-errors -Werror=implicit-function-declaration -Werror=vla -Wall -Wextra -Wpedantic -Wshadow

bf2c: main.c bf2c_basics.h
	$(CC) $(COMMON_FLAGS) -DNDEBUG -s -fstrict-aliasing -O3 -ffast-math -Wstrict-aliasing -Wno-maybe-uninitialized -Wno-unused-variable -Wno-unused-function -Wno-unused-parameter -Wno-unused-label -Wno-parentheses -o $@ main.c

bf2c_test: main.c bf2c_basics.h
	$(CC) $(COMMON_FLAGS) -fsanitize=address,undefined -Og -g -fno-strict-aliasing -o $@ main.c
