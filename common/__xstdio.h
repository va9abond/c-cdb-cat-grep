#ifndef __XSTDIO_GUARD
#define __XSTDIO_GUARD

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

// extern int XGETLINE_ERROR = 0;

ssize_t xgetline (char **bufp, size_t *bufsz, FILE *stream);




#endif // __XSTDIO_GUARD
