#include "bf2c_basics.h"
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	int c_int;
	int result;

	if (EOF == fputs(STRING_BEGIN, stdout))
		return EXIT_FAILURE;

	while (EOF != (c_int = fgetc(stdin))) {
		switch ((char)c_int) {
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
			result = ~EOF;
			break;
		}

		if (result == EOF)
			return EXIT_FAILURE;
	}

	if (EOF == fputs(STRING_END, stdout))
		return EXIT_FAILURE;

	if (EOF == fflush(stdout))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
