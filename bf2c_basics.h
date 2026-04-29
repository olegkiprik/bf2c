#ifndef BF2C_BASICS_H_SENTRY
#define BF2C_BASICS_H_SENTRY

#define STRING_BEGIN                                                           \
	"#define EFINE_HELPERS_IMPLEMENTATION\n"                               \
	"#define EFINE_FSYNC_IMPLEMENTATION\n"                                 \
	"#include \"efine_fsync.h\"\n"                                         \
	"#include \"efine_helpers.h\"\n"                                       \
	"#include <limits.h>\n"                                                \
	"#include <stddef.h>\n"                                                \
	"#include <stdio.h>\n"                                                 \
	"#include <stdlib.h>\n"                                                \
	""                                                                     \
	"#if CHAR_BIT != 8\n"                                                  \
	"#warning \"Bytes are not octets\"\n"                                  \
	"#endif\n"                                                             \
	""                                                                     \
	"#define MEMORY_INITIAL_NR_BYTES 0x1000\n"                             \
	""                                                                     \
	"EFINE_DEF int memory_double(unsigned char *restrict old_ptr,"         \
	"unsigned char *restrict old_p,"                                       \
	"unsigned char **restrict out_new_ptr,"                                \
	"unsigned char **restrict out_new_p,"                                  \
	"unsigned char **restrict "                                            \
	"out_new_first_outside,"                                               \
	"ptrdiff_t old_size)"                                                  \
	"{"                                                                    \
	"unsigned char *tmp_ptr;"                                              \
	"ptrdiff_t p_diff;"                                                    \
	""                                                                     \
	"p_diff = old_p - old_ptr;"                                            \
	""                                                                     \
	"tmp_ptr = realloc(old_ptr, old_size * 2);"                            \
	"if (tmp_ptr == NULL)"                                                 \
	"return 1;"                                                            \
	""                                                                     \
	"memset(tmp_ptr + old_size, (int)'\\0', old_size * 2);"                \
	"*out_new_ptr = tmp_ptr;"                                              \
	"*out_new_p = tmp_ptr + p_diff;"                                       \
	"*out_new_first_outside = tmp_ptr + old_size * 2;"                     \
	"return 0;"                                                            \
	"}"                                                                    \
	""                                                                     \
	"int main(void)"                                                       \
	"{"                                                                    \
	"unsigned char *memory;"                                               \
	"unsigned char *first_outside;"                                        \
	"unsigned char *p;"                                                    \
	"int c_int;"                                                           \
	"int main_result;"                                                     \
	""                                                                     \
	"main_result = EXIT_SUCCESS;"                                          \
	""                                                                     \
	"memory = malloc(MEMORY_INITIAL_NR_BYTES);"                            \
	"if (memory == NULL)"                                                  \
	"return EXIT_FAILURE;"                                                 \
	""                                                                     \
	"memset(memory, (int)'\\0', MEMORY_INITIAL_NR_BYTES);"                 \
	"p = memory;"                                                          \
	"first_outside = memory + MEMORY_INITIAL_NR_BYTES;"                    \
	""                                                                     \
	"flockfile(stdin);"                                                    \
	"flockfile(stdout);"

#define STRING_END                                                             \
	"l_finish:"                                                            \
	""                                                                     \
	"if (0 != fflush_unlocked(stdout))"                                    \
	"main_result = EXIT_FAILURE;"                                          \
	""                                                                     \
	"if (main_result != EXIT_FAILURE)"                                     \
	"if (0 != efine_fsync(STDOUT_FILENO))"                                 \
	"main_result = EXIT_FAILURE;"                                          \
	""                                                                     \
	"funlockfile(stdout);"                                                 \
	"funlockfile(stdin);"                                                  \
	"free(memory);"                                                        \
	"return main_result;"                                                  \
	""                                                                     \
	"l_memory_error:"                                                      \
	""                                                                     \
	"free(memory);"                                                        \
	"memory=NULL;"                                                         \
	"main_result=EXIT_FAILURE;"                                            \
	"goto l_finish;"                                                       \
	"}\n"

#define STRING_PLUS "++*p;"

#define STRING_MINUS "--*p;"

#define STRING_LEFT "--p;"

#define STRING_JUMP_FORWARD_IF_ZERO "while(*p!='\\0'){"

#define STRING_JUMP_BACK_IF_NOT_ZERO "}"

#define STRING_IN                                                              \
	"c_int=efine_fgetc_unlocked_no_eintr(stdin);"                          \
	"if(c_int==EOF){"                                                      \
	"assert(main_result==EXIT_SUCCESS);"                                   \
	"goto l_finish;"                                                       \
	"}"                                                                    \
	"*p=(unsigned char)c_int;"

#define STRING_OUT                                                             \
	"if(EOF==efine_fputc_unlocked_no_eintr(*p,stdout)){"                   \
	"main_result=EXIT_FAILURE;"                                            \
	"goto l_finish;"                                                       \
	"}"

#define STRING_RIGHT                                                           \
	"if(p+1==first_outside)"                                               \
	"if(0!=memory_double(memory,p,&memory,&p,&first_outside,"              \
	"first_outside-memory))"                                               \
	"goto l_memory_error;"                                                 \
	"++p;"

#endif /* BF2C_BASICS_H_SENTRY */
