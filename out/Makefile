COMMON_FLAGS = -pedantic-errors -Werror=implicit-function-declaration -Werror=vla -Wall -Wextra -Wpedantic -Wshadow

bf: out.c efine_helpers.h efine_portability_base.h efine_fsync.h
	$(CC) $(COMMON_FLAGS) -DNDEBUG -s -fstrict-aliasing -O3 -ffast-math -Wstrict-aliasing -Wno-unused-variable -Wno-unused-function -Wno-unused-parameter -Wno-unused-label -Wno-parentheses -Wno-misleading-indentation -o $@ out.c

bf_test: out.c efine_helpers.h efine_portability_base.h efine_fsync.h
	$(CC) $(COMMON_FLAGS) -fsanitize=address,undefined -Og -g -fno-strict-aliasing -o $@ out.c
