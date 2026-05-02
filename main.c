#include "bf2c_basics.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(EFINE_DEF)
#define EFINE_DEF static
#endif

#if defined(NDEBUG)
#define EFINE_NDEBUG (1 == 1)
#else
#define EFINE_NDEBUG (0 == 1)
#endif

#define EFINE_ASSUME(x)                                                                                                \
	do {                                                                                                           \
		if (EFINE_NDEBUG) {                                                                                    \
			if (!(x)) {                                                                                    \
				__builtin_unreachable();                                                               \
			}                                                                                              \
		} else {                                                                                               \
			assert(x);                                                                                     \
		}                                                                                                      \
	} while (0 == 1)

#define MEMORY_CHUNK_NR_BYTES 0x1000
#if MEMORY_CHUNK_NR_BYTES % 0x80 != 0
#warning "Risky value"
#endif

EFINE_DEF int bf2c_generate(char *p, ptrdiff_t n)
{
	int result;

	if (EOF == fputs(STRING_BEGIN, stdout))
		goto l_failed_output;

	for (; n != 0; --n, ++p) {
		switch (*p) {
		case '+':
			result = fputs(STRING_PLUS, stdout);
			break;
		case '-':
			result = fputs(STRING_MINUS, stdout);
			break;
		case ',':
			result = fputs(STRING_IN, stdout);
			break;
		case '.':
			result = fputs(STRING_OUT, stdout);
			break;
		case '<':
			result = fputs(STRING_LEFT, stdout);
			break;
		case '>':
			result = fputs(STRING_RIGHT, stdout);
			break;
		case '[':
			result = fputs(STRING_JUMP_FORWARD_IF_ZERO, stdout);
			break;
		case ']':
			result = fputs(STRING_JUMP_BACK_IF_NOT_ZERO, stdout);
			break;
		default:
			result = EOF ^ 1;
			break;
		}

		if (result == EOF)
			goto l_failed_output;
	}

	if (EOF == fputs(STRING_END, stdout))
		goto l_failed_output;

	if (EOF == fflush(stdout))
		goto l_failed_flush;

	return EXIT_SUCCESS;
l_failed_output:
	(void)fputs("\nFailed to print\n", stderr);
	return EXIT_FAILURE;
l_failed_flush:
	(void)fputs("\nFailed to flush\n", stderr);
	return EXIT_FAILURE;
}

EFINE_DEF int memory_double(void **restrict out_new_ptr, void **restrict out_new_curr,
			    void **restrict out_new_first_outside, void *old_ptr, void *old_curr, ptrdiff_t old_size,
			    int zero_new)
{
	void *tmp_ptr;
	ptrdiff_t curr_diff;

	curr_diff = (char *)old_curr - (char *)old_ptr;

	tmp_ptr = realloc(old_ptr, old_size * 2);
	if (tmp_ptr == NULL)
		return 1;

	if (zero_new == 1)
		memset((char *)tmp_ptr + old_size, '\0', old_size);

	*out_new_ptr = tmp_ptr;
	*out_new_curr = (char *)tmp_ptr + curr_diff;
	*out_new_first_outside = (char *)tmp_ptr + old_size * 2;
	return 0;
}

/*
	1 if any.
	all ignored characters are null
*/
EFINE_DEF int in_place_byte_arith_redundancy_elide(char *first, char *data_first_outside)
{
	char *curr;
	char c;
	char *p;
	int any;

	int byte_arithmetic_sum;
	int minus;
	int plus_occurred;
	int minus_occurred;

	int state;

	any = 0;
	state = 0;
	byte_arithmetic_sum = 0;
	plus_occurred = 0;
	minus_occurred = 0;

	curr = first;
	for (; curr != data_first_outside; ++curr) {
		c = *curr;

		switch (c) {
		case '[':
		case ']':
		case ',':
		case '.':
		case '>':
		case '<':
			if (state == 1) {
				minus = 0;
				if (byte_arithmetic_sum > 0x80) {
					byte_arithmetic_sum = 0x100 - byte_arithmetic_sum;
					minus = 1;
				}
				for (p = curr - byte_arithmetic_sum; p != curr; ++p)
					*p = minus == 1 ? '-' : '+';

				if (plus_occurred == 1 && minus_occurred == 1)
					any = 1;

				plus_occurred = 0;
				minus_occurred = 0;
			}
			byte_arithmetic_sum = 0;
			state = 0;
			break;
		case '+':
			state = 1;
			*curr = '\0';
			plus_occurred = 1;
			if (byte_arithmetic_sum == 0xFF)
				byte_arithmetic_sum = 0;
			else
				++byte_arithmetic_sum;
			break;
		case '-':
			state = 1;
			*curr = '\0';
			minus_occurred = 1;
			if (byte_arithmetic_sum == 0)
				byte_arithmetic_sum = 0xff;
			else
				--byte_arithmetic_sum;
			break;
		case '\0':
			break;
		default:
			EFINE_ASSUME(0 == 1);
			break;
		}
	}

	if (state == 1) {
		minus = 0;
		if (byte_arithmetic_sum > 0x80) {
			byte_arithmetic_sum = 0x100 - byte_arithmetic_sum;
			minus = 1;
		}
		for (p = curr - byte_arithmetic_sum; p != curr; ++p)
			*p = minus == 1 ? '-' : '+';

		if (plus_occurred == 1 && minus_occurred == 1)
			any = 1;
	}

	return any;
}

/*
	1 if any.
	all ignored characters are null
*/
EFINE_DEF int in_place_ptr_arith_redundancy_elide(char *first, char *data_first_outside)
{
	char *curr;
	char c;
	char *p;
	int any;

	ptrdiff_t ptr_arithmetic_sum;
	int minus;
	int plus_occurred;
	int minus_occurred;

	int state;

	any = 0;
	state = 0;
	ptr_arithmetic_sum = 0;
	plus_occurred = 0;
	minus_occurred = 0;

	curr = first;
	for (; curr != data_first_outside; ++curr) {
		c = *curr;

		switch (c) {
		case '[':
		case ']':
		case ',':
		case '.':
		case '+':
		case '-':
			if (state == 1) {
				minus = ptr_arithmetic_sum < 0 ? 1 : 0;
				if (minus == 1)
					ptr_arithmetic_sum = -ptr_arithmetic_sum;
				for (p = curr - ptr_arithmetic_sum; p != curr; ++p)
					*p = minus == 1 ? '<' : '>';

				if (plus_occurred == 1 && minus_occurred == 1)
					any = 1;

				plus_occurred = 0;
				minus_occurred = 0;
			}
			ptr_arithmetic_sum = 0;
			state = 0;
			break;
		case '>':
			state = 1;
			*curr = '\0';
			plus_occurred = 1;
			++ptr_arithmetic_sum;
			break;
		case '<':
			state = 1;
			*curr = '\0';
			minus_occurred = 1;
			--ptr_arithmetic_sum;
			break;
		case '\0':
			break;
		default:
			EFINE_ASSUME(0 == 1);
			break;
		}
	}

	if (state == 1) {
		minus = ptr_arithmetic_sum < 0 ? 1 : 0;
		if (minus == 1)
			ptr_arithmetic_sum = -ptr_arithmetic_sum;
		for (p = curr - ptr_arithmetic_sum; p != curr; ++p)
			*p = minus == 1 ? '<' : '>';

		if (plus_occurred == 1 && minus_occurred == 1)
			any = 1;
	}

	return any;
}

/*
	all ignored characters are null
*/
EFINE_DEF int in_place_byte_arith_before_input_elide(char *first, char *data_first_outside)
{
	char *curr;
	char *p;
	ptrdiff_t tmp_dist;
	ptrdiff_t bracket_level;
	char *from;
	int any;

	any = 0;
	tmp_dist = 0;
	bracket_level = 0;

	for (curr = first; curr != data_first_outside; ++curr) {
		if (*curr == '<' || *curr == '>' || *curr == '.') {
			tmp_dist = 0;
		} else if (*curr == ',') {
			p = curr;
			from = curr;
			while (tmp_dist != 0) {
				--p;

				if (*p == ']')
					++bracket_level;
				else if (*p == '[')
					--bracket_level;

				if (bracket_level == -1)
					break;

				if (bracket_level == 0)
					from = p;

				--tmp_dist;
			}
			for (p = from; p != curr; ++p) {
				assert(*p != '.' && *p != ',' && *p != '<' && *p != '>');
				if (*p == '+' || *p == '-' || *p == '[' || *p == ']')
					any = 1;
				*p = '\0';
			}

			tmp_dist = 0;
			bracket_level = 0;
		} else {
			++tmp_dist;
		}
	}

	return any;
}

/*
	all ignored characters are null
*/
EFINE_DEF int in_place_after_io_elide(char *first, char *data_first_outside)
{
	char *p;
	ptrdiff_t bracket_level;
	char *from;
	int any;

	any = 0;
	bracket_level = 0;
	p = data_first_outside;
	from = data_first_outside;

	while (p != first) {
		--p;

		if (*p == ']')
			++bracket_level;
		else if (*p == '[')
			--bracket_level;
		else if (*p == '.' || *p == ',')
			break;

		if (bracket_level == 0)
			from = p;
	}

	if (p == first && *p != '.' && *p != ',')
		from = first;

	for (p = from; p != data_first_outside; ++p) {
		assert(*p != '.' && *p != ',');
		if (*p == '+' || *p == '-' || *p == '[' || *p == ']' || *p == '<' || *p == '>')
			any = 1;
		*p = '\0';
	}

	return any;
}

/*
	brackets must be correct
	all ignored characters are null
*/
EFINE_DEF int in_place_loops_before_modifications_elide(char *first, char *data_first_outside)
{
	int any;
	char *curr;
	ptrdiff_t bracket_level;
	char *from;
	char *p;

	any = 0;
	bracket_level = 0;

	for (curr = first; curr != data_first_outside; ++curr) {
		if (*curr == '[') {
			if (bracket_level == 0)
				from = curr;
			++bracket_level;
		} else if (*curr == ']') {
			--bracket_level;
			if (bracket_level == 0) {
				for (p = from; p != curr; ++p)
					*p = '\0';
				*curr = '\0';
				any = 1;
			}
		} else if (*curr == '.' || *curr == ',' || *curr == '+' || *curr == '-') {
			if (bracket_level == 0)
				break;
		} else if (*curr != '<' && *curr != '>' && *curr != '\0') {
			EFINE_ASSUME(0 == 1);
		}
	}

	return any;
}

/*
	all ignored characters are null
*/
EFINE_DEF int in_place_loops_after_zero_elide(char *first, char *data_first_outside)
{
	char *curr;
	ptrdiff_t bracket_level;
	int any;
	int elide;

	any = 0;
	bracket_level = 0;
	elide = 0;

	for (curr = first; curr != data_first_outside; ++curr) {
		if (*curr == '\0') {
			/* ignore */
		} else {
			if (elide == 1) {
				if (*curr == '[') {
					++bracket_level;
				} else if (*curr == ']') {
					--bracket_level;
					if (bracket_level == -1) {
						elide = 0;
						bracket_level = 0;
					}
				} else {
					if (bracket_level == 0)
						elide = 0;
				}
			}

			if (elide == 1) {
				*curr = '\0';
				any = 1;
			}

			if (*curr == ']')
				elide = 1;
		}
	}

	return any;
}

/*
	brackets must be correct.
	all ignored characters are null.
	no infinite loops
*/
EFINE_DEF int in_place_simplify_zeroing_loops(int *out_any, char *first, char *data_first_outside)
{
	void *bracket_stack;
	void *bracket_stack_buff_first_outside;
	void *bracket_stack_head; /* first outside of data */

	int any;
	char *curr;
	char **tmp_sp;
	char *p;

	any = 0;
	bracket_stack = malloc(MEMORY_CHUNK_NR_BYTES);
	if (bracket_stack == NULL)
		return 1;

	bracket_stack_buff_first_outside = (char *)bracket_stack + MEMORY_CHUNK_NR_BYTES;
	bracket_stack_head = bracket_stack;

	for (curr = first; curr != data_first_outside; ++curr) {
		switch (*curr) {
		case ',':
		case '.':
		case '>':
		case '<':
			for (tmp_sp = bracket_stack; tmp_sp != bracket_stack_head; ++tmp_sp)
				*tmp_sp = NULL;
			break;
		case '[':
			*(char **)bracket_stack_head = curr;
			bracket_stack_head = (char **)bracket_stack_head + 1;
			if (bracket_stack_head == bracket_stack_buff_first_outside)
				if (0 != memory_double(
					     &bracket_stack, &bracket_stack_head, &bracket_stack_buff_first_outside,
					     bracket_stack, bracket_stack_head,
					     (char *)bracket_stack_buff_first_outside - (char *)bracket_stack, 0)) {
					free(bracket_stack);
					return 1;
				}
			break;
		case ']':
			assert(bracket_stack_head != bracket_stack);
			tmp_sp = (char **)bracket_stack_head - 1;
			if (*tmp_sp != NULL) {
				if (*tmp_sp + 2 != curr || *(*tmp_sp + 1) != '-')
					any = 1;

				for (p = *tmp_sp + 1; p != curr; ++p)
					*p = '\0';
				assert(*tmp_sp + 1 != curr);
				*(*tmp_sp + 1) = '-';
			}
			bracket_stack_head = (char **)bracket_stack_head - 1;
			break;
		case '\0':
		case '+':
		case '-':
			break;
		default:
			EFINE_ASSUME(0 == 1);
			break;
		}
	}

	*out_any = any;
	free(bracket_stack);
	return 0;
}

/*
	[]
	all ignored characters are null.
	0 = no infinite loops
*/
EFINE_DEF int check_infinite_loops(char *first, char *data_first_outside)
{
	char *curr;
	char tmp;

	tmp = '\0';

	for (curr = first; curr != data_first_outside; ++curr) {
		if (tmp == '[' && *curr == ']')
			return 1;
		if (*curr != '\0')
			tmp = *curr;
	}

	return 0;
}

EFINE_DEF int check_brackets_ok(char *first, char *data_first_outside)
{
	ptrdiff_t bracket_level;
	char *curr;

	bracket_level = 0;

	for (curr = first; curr != data_first_outside; ++curr) {
		if (*curr == '[')
			++bracket_level;
		else if (*curr == ']') {
			--bracket_level;
			if (bracket_level == -1)
				return 0;
		}
	}

	return bracket_level == 0 ? 1 : 0;
}

/*
	all ignored characters are null.
	brackets must be correct
*/
EFINE_DEF int in_place_bracket_redundancy_elide(int *out_any, char *first, char *data_first_outside)
{
	void *bracket_stack;
	void *bracket_stack_buff_first_outside;
	void *bracket_stack_head; /* first outside of data */

	int any;
	char *curr;
	char **tmp_sp;

	any = 0;
	bracket_stack = malloc(MEMORY_CHUNK_NR_BYTES);
	if (bracket_stack == NULL)
		return 1;

	bracket_stack_buff_first_outside = (char *)bracket_stack + MEMORY_CHUNK_NR_BYTES;
	bracket_stack_head = bracket_stack;

	for (curr = first; curr != data_first_outside; ++curr) {
		switch (*curr) {
		case ',':
		case '.':
		case '+':
		case '-':
		case '>':
		case '<':
			if (bracket_stack_head == bracket_stack)
				break;

			tmp_sp = (char **)bracket_stack_head - 1;
			*tmp_sp = NULL;
			break;
		case '[':
			*(char **)bracket_stack_head = curr;
			bracket_stack_head = (char **)bracket_stack_head + 1;
			if (bracket_stack_head == bracket_stack_buff_first_outside) {
				if (0 != memory_double(
					     &bracket_stack, &bracket_stack_head, &bracket_stack_buff_first_outside,
					     bracket_stack, bracket_stack_head,
					     (char *)bracket_stack_buff_first_outside - (char *)bracket_stack, 0)) {
					free(bracket_stack);
					return 1;
				}
			}
			break;
		case ']':
			assert(bracket_stack_head != bracket_stack);
			tmp_sp = (char **)bracket_stack_head - 1;
			if (*tmp_sp != NULL) {
				*curr = '\0';
				**tmp_sp = '\0';
				any = 1;
			}
			bracket_stack_head = (char **)bracket_stack_head - 1;
			break;
		case '\0':
			break;
		default:
			EFINE_ASSUME(0 == 1);
			break;
		}
	}

	*out_any = any;
	free(bracket_stack);
	return 0;
}

/*
	<[<]>[>] == [>]
	ignored symbols must be null
*/
EFINE_DEF int in_place_round_trip_elide(char *first, char *data_first_outside)
{
	int any;
	char *curr;
	ptrdiff_t magnitude;
	int state;
	ptrdiff_t tmp_magn;
	char *from;
	char *to_outside;

	state = 0;
	any = 0;
	magnitude = 0;
	from = NULL;

	for (curr = first; curr != data_first_outside; ++curr) {
		switch (state) {
		case 0:
			if (*curr == '>') {
				++magnitude;
			} else if (*curr == '<') {
				--magnitude;
			} else if (*curr == '[') {
				if (magnitude != 0) {
					state = 1;
					tmp_magn = 0;
				}
			} else if (*curr != '\0') {
				magnitude = 0;
			}

			if (*curr == '<' || *curr == '>')
				if (from == NULL)
					from = curr;

			break;
		case 1:
			goto l_check_magnitude;
		case 2:
			if (*curr == ']') {
				state = 3;
				tmp_magn = 0;
			} else if (*curr != '\0') {
				state = 0;
				magnitude = 0;
			}
			break;
		case 3:
			goto l_check_magnitude;
		case 4:
			if (*curr == '[') {
				to_outside = curr;
				state = 5;
				tmp_magn = 0;
			} else if (*curr != '\0') {
				state = 0;
				magnitude = 0;
			}
			break;
		case 5:
			goto l_check_magnitude;
		case 6:
			if (*curr == ']') {
				/* Round trip detected! */
				memset(from, '\0', to_outside - from);
				any = 1;
			}
			state = 0;
			magnitude = 0;
			break;
		default:
			assert(0 == 1);
			break;
		}

		if (state == 0 && magnitude == 0)
			from = NULL;

		continue;
	l_check_magnitude:

		if (state == 1 && magnitude > 0 || state != 1 && magnitude < 0) {
			if (*curr == '>') {
				++tmp_magn;
			} else if (state == 1 && *curr == ']') {
				while (magnitude != tmp_magn) {
					if (*from == '>')
						--magnitude;
					++from;
				}
				--curr;
			} else if (*curr != '\0') {
				state = 0;
				if (*curr == '<') {
					magnitude = -1;
					from = curr;
				}
			}
		} else {
			if (*curr == '<') {
				--tmp_magn;
			} else if (state == 1 && *curr == ']') {
				while (magnitude != tmp_magn) {
					if (*from == '<')
						++magnitude;
					++from;
				}
				--curr;
			} else if (*curr != '\0') {
				state = 0;
				if (*curr == '>') {
					magnitude = 1;
					from = curr;
				}
			}
		}
		if (state == 1 && tmp_magn == magnitude || state != 1 && tmp_magn == -magnitude) {
			if (state == 1)
				state = 2;
			else if (state == 3)
				state = 4;
			else if (state == 5)
				state = 6;
		}
	}

	return any;
}

EFINE_DEF void in_place_ignored_to_null(char *p, char *data_first_outside)
{
	for (; p != data_first_outside; ++p) {
		if (*p != ',' && *p != '.' && *p != '[' && *p != ']' && *p != '+' && *p != '-' && *p != '<' &&
		    *p != '>')
			*p = '\0';
	}
}

/*
	<[<]>[>] == [>]
	ignored symbols must be null.
	returns new data outside
*/
EFINE_DEF char *in_place_fit_code(char *first, char *data_first_outside)
{
	char *out;
	char *in;

	in = first;
	for (out = first; out != data_first_outside; ++out)
		if (*out != '\0') {
			*in = *out;
			++in;
		}

	return in;
}

int main(void)
{
	void *memory_handle;
	void *buffer_first_outside;
	void *p;

	char *memory;
	char *data_first_outside;

	size_t bytes_read;
	int any_elis_ptr;
	int any_elis_byte;
	int any;

	memory_handle = malloc(MEMORY_CHUNK_NR_BYTES);
	if (memory_handle == NULL)
		return EXIT_FAILURE;

	p = memory_handle;
	buffer_first_outside = (char *)memory_handle + MEMORY_CHUNK_NR_BYTES;

	flockfile(stdin);

	/* read the whole file */

	while (MEMORY_CHUNK_NR_BYTES == (bytes_read = fread_unlocked(p, 1, MEMORY_CHUNK_NR_BYTES, stdin))) {
		p = (char *)p + MEMORY_CHUNK_NR_BYTES;

		if (p == buffer_first_outside) {
			if (0 != memory_double(&memory_handle, &p, &buffer_first_outside, memory_handle, p,
					       (char *)buffer_first_outside - (char *)memory_handle, 0))
				goto l_failed_memory_when_reading;
		}
	}

	funlockfile(stdin);

	if (0 != ferror(stdin))
		goto l_failed_read;

	p = (char *)p + bytes_read;

	memory = memory_handle;
	data_first_outside = p;

	in_place_ignored_to_null(memory, data_first_outside);

	/* optimizations */

	while (1 == 1) {
		any_elis_ptr = in_place_ptr_arith_redundancy_elide(memory, data_first_outside);
		any_elis_byte = in_place_byte_arith_redundancy_elide(memory, data_first_outside);
		if (any_elis_ptr == 0 && any_elis_byte == 0)
			break;
	}

	(void)in_place_byte_arith_before_input_elide(memory, data_first_outside);
	(void)in_place_loops_after_zero_elide(memory, data_first_outside);
	(void)in_place_after_io_elide(memory, data_first_outside);

	if (1 != check_brackets_ok(memory, data_first_outside))
		goto l_wrong_input;

	if (0 != in_place_bracket_redundancy_elide(&any, memory, data_first_outside))
		goto l_failed_memory;

	assert(1 == check_brackets_ok(memory, data_first_outside));
	(void)in_place_loops_before_modifications_elide(memory, data_first_outside);

	(void)in_place_ptr_arith_redundancy_elide(memory, data_first_outside);

	if (0 != check_infinite_loops(memory, data_first_outside))
		goto l_wrong_input;

	if (0 != in_place_simplify_zeroing_loops(&any, memory, data_first_outside))
		goto l_failed_memory;

	while (1 == in_place_round_trip_elide(memory, data_first_outside))
		;

	/* finish optimizations */

	data_first_outside = in_place_fit_code(memory, data_first_outside);

	/* either fit the result or set ignored characters to ' ' */

	if (EOF == fputs("/*\n", stdout))
		goto l_failed_write;

	flockfile(stdout);
	if (data_first_outside - memory != (ptrdiff_t)fwrite_unlocked(memory, 1, data_first_outside - memory, stdout))
		goto l_failed_fwrite;
	funlockfile(stdout);

	if (EOF == fputs("\n*/\n", stdout))
		goto l_failed_write;

	if (0 != bf2c_generate(memory, data_first_outside - memory))
		goto l_failed_write;

	free(memory_handle);

	if (EOF == fflush(stdout))
		goto l_failed_flush;

	return EXIT_SUCCESS;

l_failed_memory:
	free(memory_handle);
	(void)fputs("\nFailed to allocate memory while optimizing\n", stderr);
	return EXIT_FAILURE;

l_wrong_input:
	free(memory_handle);
	(void)fputs("\nWrong input\n", stderr);
	return EXIT_FAILURE;

l_failed_memory_when_reading:
	funlockfile(stdin);
	free(memory_handle);
	(void)fputs("\nFailed to allocate memory while reading\n", stderr);
	return EXIT_FAILURE;

l_failed_read:
	free(memory_handle);
	(void)fputs("\nFailed to read\n", stderr);
	return EXIT_FAILURE;

l_failed_write:
	free(memory_handle);
	(void)fputs("\nFailed to write the result\n", stderr);
	return EXIT_FAILURE;

l_failed_fwrite:
	funlockfile(stdout);
	free(memory_handle);
	(void)fputs("\nFailed to write a temp\n", stderr);
	return EXIT_FAILURE;

l_failed_flush:
	(void)fputs("\nFailed to flush\n", stderr);
	return EXIT_FAILURE;
}
