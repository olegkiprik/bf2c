/*
	Define EFINE_NO_UNNECESSARY_DEPENDING_ON_UNISTD if necessary
*/

#if !defined(EFINE_FSYNC_H_SENTRY)
#define EFINE_FSYNC_H_SENTRY
#include "efine_helpers.h"

EFINE_DEF int efine_fsync(int fd);

#if defined(EFINE_FSYNC_IMPLEMENTATION)

#include <errno.h>

#if !defined(EFINE_NO_UNNECESSARY_DEPENDING_ON_UNISTD)
#include <unistd.h>
#endif

EFINE_DEF int efine_fsync(int fd)
{
#if !defined(EFINE_NO_UNNECESSARY_DEPENDING_ON_UNISTD)
	if (0 != fsync(fd) && errno != EINVAL) {
		return 1;
	}
#endif
	return 0;
}

#endif /* EFINE_FSYNC_IMPLEMENTATION */

#endif /* EFINE_FSYNC_H_SENTRY */

/* 2026-04-16 */
