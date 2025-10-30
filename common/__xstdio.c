#include <__xstdio.h>

#ifndef XLINESIZE
#define XLINESIZE 512
#endif // XLINESIZE

ssize_t xgetline (char **bufp, size_t *bufsz, FILE *stream) {
    if (bufp == NULL || bufsz == NULL || stream == NULL)
        return -1;

    if (*bufsz == 0 || *bufp == NULL) {
        *bufsz = XLINESIZE;
        *bufp = (char*) malloc(*bufsz * sizeof(char));

        if (bufp == NULL) // TODO msg: bad alloc
            return -1;
    }

    size_t pos = 0;
    int ch = 0;
    while (ferror(stream) == 0 && (ch = fgetc(stream)) != '\n' && ch != EOF) {

        if (pos+1 >= *bufsz) { // expand space
            size_t new_n = *bufsz * 2;
            char *new_linep = realloc(*bufp, new_n);
            if (bufp == NULL)
                return -1;

            *bufp = new_linep;
            *bufsz = new_n;
        }

        (*bufp)[pos++] = ch;
    }

    if (ch == '\n')
        (*bufp)[pos++] = ch;
    (*bufp)[pos] = '\0';

    return ferror(stream) == 0 ? (ssize_t)pos : -1;
}

