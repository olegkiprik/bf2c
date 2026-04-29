/*
	Helpers to make C more dependable
*/

#if !defined(EFINE_PORTABILITY_BASE_H_SENTRY)
#define EFINE_PORTABILITY_BASE_H_SENTRY

#if defined(EFINE_NO_UNNECESSARY_DEPENDING_ON_UNISTD)
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#define F_OK 0
#define W_OK 2
#define R_OK 4
#endif

#if !defined(EFINE_NOT_SUPPORTED_UNLOCKED_IO)
#endif

#if !defined(EFINE_NOT_SUPPORTED_ALIGNED_ALLOC)
#endif

#if !defined(EFINE_FLAT_MEMORY_MODEL)
#endif

#if !defined(EFINE_TWO_S_COMPLEMENT)
#endif

#if defined(EFINE_NOT_SUPPORTED_STDINT)
#endif

#if !defined(EFINE_REPLACED_UNISTD)
#endif

#if defined(_WIN32) && !defined(EFINE_REPLACED_UNISTD)
#error "Please provide a replacement for unistd.h"
#endif

#if !defined(EFINE_ASSUME_ORDERED_ASCII)
#endif

#if defined(EFINE_NOT_SUPPORTED_BUILTIN)
#define __builtin_unreachable()                                                                                        \
	do {                                                                                                           \
	} while (0 == 1)
#define __builtin_expect(a, b) ((a) == (b))
#define __builtin_expect_with_probability(a, b, c) ((a) == (b))
#define __builtin_assume_aligned(a, b, c) (a)
#define __builtin_prefetch(addr, rw, locality) (addr)
#endif

#if defined(EFINE_NOT_SUPPORTED_UNLOCKED_IO)

#define clearerr_unlocked(stream) clearerr(stream)
#define fileno_unlocked(stream) fileno(stream)
#define feof_unlocked(stream) feof(stream)
#define ferror_unlocked(stream) ferror(stream)
#define fflush_unlocked(stream) fflush(stream)
#define fgetc_unlocked(stream) fgetc(stream)
#define fputc_unlocked(c, stream) fputc(c, stream)
#define fread_unlocked(ptr, size, n, stream) fread(ptr, size, n, stream)
#define fwrite_unlocked(ptr, size, n, stream) fwrite(ptr, size, n, stream)
#define getc_unlocked(stream) getc(stream)
#define getchar_unlocked() getchar()
#define putc_unlocked(c, stream) putc(c, stream)
#define putchar_unlocked(c) putchar(c)

#define flockfile(filehandle)                                                                                          \
	do {                                                                                                           \
	} while (0 == 1)
#define ftrylockfile(filehandle) 0
#define funlockfile(filehandle)                                                                                        \
	do {                                                                                                           \
	} while (0 == 1)

#endif /* EFINE_NOT_SUPPORTED_UNLOCKED_IO */

/*
	https://dependablec.org

	typedef struct MyStringType_ {
		int string_length;
		char string[DEPENDABLE_C_FLEXIBLE_ARRAY_MEMBER];
	} MyStringType;

	s = malloc((sizeof *s) + sizeof(char) * (i - DEPENDABLE_C_FLEXIBLE_ARRAY_MEMBER_SUBTRACT));
*/
#if __STDC_VERSION__ >= 199901L
#define EFINE_FLEXIBLE_ARRAY_MEMBER
#define EFINE_FLEXIBLE_ARRAY_MEMBER_SUBTRACT 0
#elif defined(__clang__) || defined(__GNUC__)
#define EFINE_FLEXIBLE_ARRAY_MEMBER 0
#define EFINE_FLEXIBLE_ARRAY_MEMBER_SUBTRACT 0
#else
#define EFINE_FLEXIBLE_ARRAY_MEMBER 1
#define EFINE_FLEXIBLE_ARRAY_MEMBER_SUBTRACT 1
#endif

#endif /* EFINE_PORTABILITY_BASE_H_SENTRY */

/* 2026-04-23 */
