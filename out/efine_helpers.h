/*
	Helpers to make C more dependable
*/

#if !defined(EFINE_HELPERS_H_SENTRY)
#define EFINE_HELPERS_H_SENTRY
#include <assert.h>
#include <stddef.h>

/* static or extern */
#if !defined(EFINE_DEF)
#define EFINE_DEF static
#endif

/* returns 0 or 1 */
#define EFINE_DO_NOT_CARE_0_OR_1() 0

/* sar instruction */
#define EFINE_FLOOR_DIV_BY_POW_2(type, x, n)                                                                           \
	(((x) > -1) ? ((x) / ((type)1 << (n))) : (-((-((x) + 1)) / ((type)1 << (n))) - 1))

/*
	use mem_handle in free and efine realloc.
	use mem_result in accessing allocated memory.
	n_bytes_alignment != 0 .
	n_chunks_to_allocate != 0 .

	no check for overflow

	restrict
*/
EFINE_DEF int efine_aligned_alloc(void **mem_handle, void **mem_result, size_t n_bytes_alignment,
				  size_t n_chunks_to_allocate);

/*
	old_ptr_handle != NULL

	potentially memory inefficient.
	n_bytes_alignment != 0 .
	n_chunks_to_allocate != 0 .
	prefer_stdlib_realloc is either 0 or 1 .

	no check for overflow

	restrict
*/
EFINE_DEF int efine_aligned_realloc(void **mem_new_handle, void **mem_new_result, void *old_ptr_handle,
				    size_t n_chunks_previously_allocated, size_t n_bytes_alignment,
				    size_t n_chunks_to_allocate, int prefer_stdlib_realloc);

EFINE_DEF int efine_is_zeroed(const void *p, size_t n_bytes);

/* FILE* stream */
EFINE_DEF int efine_fgetc_unlocked_no_eintr(void *stream);

/* FILE* stream */
EFINE_DEF int efine_fputc_unlocked_no_eintr(int c, void *stream);

/* FILE* stream */
EFINE_DEF int efine_fgetc_no_eintr(void *stream);

/* FILE* stream */
EFINE_DEF int efine_fputc_no_eintr(int c, void *stream);

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

#if defined(EFINE_HELPERS_IMPLEMENTATION)

#include "efine_portability_base.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(EFINE_NOT_SUPPORTED_STDINT)
#include <stdint.h>
#endif

EFINE_DEF int efine_is_zeroed(const void *p, size_t n_bytes)
{
	char *cptr;
	char *first_outside;

	cptr = (char *)p;
	first_outside = cptr + n_bytes;

	for (; cptr != first_outside; ++cptr) {
		if (*cptr != '\0')
			return 0;
	}

	return 1;
}

EFINE_DEF int efine_fgetc_unlocked_no_eintr(void *stream)
{
	int c_int;

	do {
		c_int = fgetc_unlocked((FILE *)stream);
	} while (c_int == EOF && errno == EINTR);

	return c_int;
}

EFINE_DEF int efine_fputc_unlocked_no_eintr(int c, void *stream)
{
	int result;

	do {
		result = fputc_unlocked(c, (FILE *)stream);
	} while (result == EOF && errno == EINTR);

	return result;
}

EFINE_DEF int efine_fgetc_no_eintr(void *stream)
{
	int c_int;

	do {
		c_int = fgetc((FILE *)stream);
	} while (c_int == EOF && errno == EINTR);

	return c_int;
}

EFINE_DEF int efine_fputc_no_eintr(int c, void *stream)
{
	int result;

	do {
		result = fputc(c, (FILE *)stream);
	} while (result == EOF && errno == EINTR);

	return result;
}

static void efine_priv_aligned_alloc_assume(void **restrict mem_handle, void **restrict mem_result,
					    size_t n_bytes_alignment, size_t n_chunks_to_allocate, size_t size_max)
{
	EFINE_ASSUME(mem_handle != NULL);
	EFINE_ASSUME(mem_result != NULL);
	EFINE_ASSUME(n_bytes_alignment != 0);
	EFINE_ASSUME(n_chunks_to_allocate != 0);
	EFINE_ASSUME(n_chunks_to_allocate <= size_max / n_bytes_alignment);
}

EFINE_DEF int efine_aligned_alloc(void **restrict mem_handle, void **restrict mem_result, size_t n_bytes_alignment,
				  size_t n_chunks_to_allocate)
{
	void *p;
	char *tmp_ptr;
	size_t size_max;

#if defined(EFINE_TWO_S_COMPLEMENT)
	size_max = ~(size_t)0;
#elif defined(SIZE_MAX)
	size_max = SIZE_MAX;
#else
#error "Please define SIZE_MAX"
#endif

	efine_priv_aligned_alloc_assume(mem_handle, mem_result, n_bytes_alignment, n_chunks_to_allocate, size_max);

#if !defined(EFINE_NOT_SUPPORTED_ALIGNED_ALLOC)

	p = aligned_alloc(n_bytes_alignment, n_bytes_alignment * n_chunks_to_allocate);
	if (p == NULL) {
		return 1;
	}

	*mem_handle = p;
	*mem_result = p;

#elif defined(EFINE_FLAT_MEMORY_MODEL)

	p = malloc(n_bytes_alignment * n_chunks_to_allocate + n_bytes_alignment - 1);
	if (p == NULL) {
		return 1;
	}

	*mem_handle = p;
	tmp_ptr = (char *)p;
	tmp_ptr += (n_bytes_alignment - (uintptr_t)p % n_bytes_alignment) % n_bytes_alignment;
	*mem_result = (void *)tmp_ptr;

#else
#error "Unsupported function"
#endif

	return 0;
}

static int efine_priv_aligned_realloc_flat(void **mem_new_handle, void **mem_new_result, void *old_ptr_handle,
					   size_t n_chunks_previously_allocated, size_t n_bytes_alignment,
					   size_t n_chunks_to_allocate);

EFINE_DEF int efine_aligned_realloc(void **restrict mem_new_handle, void **restrict mem_new_result,
				    void *restrict old_ptr_handle, size_t n_chunks_previously_allocated,
				    size_t n_bytes_alignment, size_t n_chunks_to_allocate, int prefer_stdlib_realloc)
{
	void *p;
	size_t size_max;
	size_t min_old_new_bytes_alloc;

#if defined(EFINE_TWO_S_COMPLEMENT)
	size_max = ~(size_t)0;
#elif defined(SIZE_MAX)
	size_max = SIZE_MAX;
#else
#error "Please define SIZE_MAX"
#endif

	efine_priv_aligned_alloc_assume(mem_new_handle, mem_new_result, n_bytes_alignment, n_chunks_to_allocate,
					size_max);
	EFINE_ASSUME(old_ptr_handle != NULL);
	EFINE_ASSUME(n_chunks_previously_allocated <= size_max / n_bytes_alignment);
	EFINE_ASSUME(prefer_stdlib_realloc == 0 || prefer_stdlib_realloc == 1);

#if !defined(EFINE_NOT_SUPPORTED_ALIGNED_ALLOC)

#if defined(EFINE_FLAT_MEMORY_MODEL)
	if (prefer_stdlib_realloc == 1) {
		if (0 != efine_priv_aligned_realloc_flat(mem_new_handle, mem_new_result, old_ptr_handle,
							 n_chunks_previously_allocated, n_bytes_alignment,
							 n_chunks_to_allocate))
			return 1;
		return 0; /* please do not delete this statement */
	}
#endif
	p = aligned_alloc(n_bytes_alignment, n_bytes_alignment * n_chunks_to_allocate);
	if (p == NULL)
		return 1;

	min_old_new_bytes_alloc =
	    n_chunks_to_allocate > n_chunks_previously_allocated ? n_chunks_previously_allocated : n_chunks_to_allocate;

	min_old_new_bytes_alloc *= n_bytes_alignment;

	memcpy(p, old_ptr_handle, min_old_new_bytes_alloc);
	free(old_ptr_handle);

	*mem_new_handle = p;
	*mem_new_result = p;

#elif defined(EFINE_FLAT_MEMORY_MODEL)

	if (0 != efine_priv_aligned_realloc_flat(mem_new_handle, mem_new_result, old_ptr_handle,
						 n_chunks_previously_allocated, n_bytes_alignment,
						 n_chunks_to_allocate))
		return 1;

#else
#error "Unsupported function"
#endif

	return 0;
}

static int efine_priv_aligned_realloc_flat(void **restrict mem_new_handle, void **restrict mem_new_result,
					   void *restrict old_ptr_handle, size_t n_chunks_previously_allocated,
					   size_t n_bytes_alignment, size_t n_chunks_to_allocate)
{
	ptrdiff_t old_alignment;
	ptrdiff_t new_alignment;
	void *old_aligned;
	void *new_aligned;
	size_t min_old_new_bytes_alloc;
	void *p;
	char *tmp_ptr;
	size_t size_max;

#if defined(EFINE_TWO_S_COMPLEMENT)
	size_max = ~(size_t)0;
#elif defined(SIZE_MAX)
	size_max = SIZE_MAX;
#else
#error "Please define SIZE_MAX"
#endif

	efine_priv_aligned_alloc_assume(mem_new_handle, mem_new_result, n_bytes_alignment, n_chunks_to_allocate,
					size_max);
	EFINE_ASSUME(old_ptr_handle != NULL);
	EFINE_ASSUME(n_chunks_previously_allocated <= size_max / n_bytes_alignment);

	old_alignment =
	    (ptrdiff_t)((n_bytes_alignment - (uintptr_t)old_ptr_handle % n_bytes_alignment) % n_bytes_alignment);

	p = realloc(old_ptr_handle, n_bytes_alignment * n_chunks_to_allocate + n_bytes_alignment - 1);
	if (p == NULL)
		return 1;

	new_alignment = (ptrdiff_t)((n_bytes_alignment - (uintptr_t)p % n_bytes_alignment) % n_bytes_alignment);

	tmp_ptr = (char *)p;
	tmp_ptr += old_alignment;
	old_aligned = (void *)tmp_ptr;

	tmp_ptr = (char *)p;
	tmp_ptr += new_alignment;
	new_aligned = (void *)tmp_ptr;

	min_old_new_bytes_alloc =
	    n_chunks_to_allocate > n_chunks_previously_allocated ? n_chunks_previously_allocated : n_chunks_to_allocate;
	min_old_new_bytes_alloc *= n_bytes_alignment;

	memmove(new_aligned, old_aligned, min_old_new_bytes_alloc);

	*mem_new_handle = p;
	*mem_new_result = new_aligned;

	return 0;
}

#endif /* EFINE_HELPERS_IMPLEMENTATION */

#endif /* EFINE_HELPERS_H_SENTRY */

/* 2026-04-23 */
