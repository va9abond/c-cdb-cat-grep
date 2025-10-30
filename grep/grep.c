/* grep */


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <__xstdio.h>


#define DEBUG 0
#define PROGRAM_NAME "grep"
#define AUTHOR "Rustem Sirazetdinov"


static char const *const OPTS_SHORT = "e:ivclnhsf:o";

static char opt_ignore_case   = 0;
static char opt_invert        = 0;
static char opt_count_only    = 0;
static char opt_files_only    = 0;
static char opt_number        = 0;
static char opt_anonymous     = 0;
static char opt_silent        = 0;
static char opt_matching_part = 0;

static char *pattern = NULL;

static size_t cnt_line = 0;

static char *grep (char *needle, char **bufp, size_t *bufsz, FILE *stream);
static char *strstr_ignore_case (const char *str, const char *subsrt);
static int   cook_opts (int argc, char **argv);
static void  usage (void);


int main (int argc, char **argv) {
    int rval = 0;

    // Validate arguments amount
    if (argc < 2) {
        usage();
        return -1;
    }

    // Parse args
    int argind = cook_opts(argc, argv);

    // If the -e option is not enabled, then we consider the first argument
    // to be a pattern
    if (pattern == NULL) {
        pattern = *(argv+argind);
        ++argind;
    }

    // Too few arguments
    if (argc - argind < 1 || pattern == NULL) {
        usage();
        return -1;
    }

    // Enable anonymous opt if a single input stream is used
    if (argind == argc-1)
        opt_anonymous = 1;

    // [WARNING] 'grep' allocate memory under the 'line' pointer,
    //           that memory must be released explicitly.
    {
        FILE   *istream = NULL;
        char   *line = NULL;
        size_t  size = 0;

        // Main loop, iterates by files
        for (char **argp = argv+argind;
                rval == 0 && *argp != NULL;
                ++argind, ++argp, cnt_line = 0) {

            if ( (*argp)[0] == '-' && (*argp)[1] == '\0' ) {
                istream = stdin;
                *argp = "(standard input)";
            } else
                istream = fopen(*argp, "r");

            if (istream == NULL) {
                if (!opt_silent)
                    printf("[ERROR] " PROGRAM_NAME ": cannot open file '%s'\n", *argp);

                continue;
            }

            size_t cnt_matches = 0;
            char *where = NULL;
            while ( feof(istream) == 0 && ferror(istream) == 0 &&
                    (where = grep(pattern, &line, &size, istream)) ) {

                if (opt_count_only || (opt_invert && opt_matching_part)) {
                    ++cnt_matches;
                    continue; // suppress normal output
                }

                if (opt_files_only) {
                    fprintf(stdout, "%s\n", *argp);
                    break; // suppress normal output
                }

                if (!opt_anonymous)
                    fprintf(stdout, "%s:", *argp);

                if (opt_number)
                    fprintf(stdout, "%lu:", cnt_line);

                if (!opt_matching_part)
                    fprintf(stdout, "%s", line);
                else
                    fprintf(stdout, "%.*s\n", (int)strlen(pattern), where);

            }
            if (opt_count_only) {
                if (!opt_anonymous)
                    fprintf(stdout, "%s:", *argp);
                fprintf(stdout, "%lu\n", cnt_matches);
            }

            if (istream != stdin)
                fclose(istream);
            else {
                clearerr(stdin); // clear feof(stdin) flag
                fflush(stdout);
            }

        }
        free(line); // [WARNING] release the memory that was allocated in 'grep'
    }

    return rval;
}


char *grep (char *needle, char **bufp, size_t *bufszp, FILE *stream) {
    if (needle == NULL || bufp == NULL || bufszp == NULL || stream == NULL)
        return NULL;

    // [WARNING] Function 'xgetline' allocates memory to store the read line,
    //           that memory must be released explicitly

    char *where = (!opt_invert) ? NULL : needle;
    while ( ((!opt_invert && where == NULL) || (opt_invert && where != NULL)) &&
            xgetline(bufp, bufszp, stream) > 0 )
    {
        ++cnt_line;

        where = (opt_ignore_case) ?
            strstr_ignore_case(*bufp, needle) : strstr(*bufp, needle);
    }
    // (Same as above but easier to read)
    // if (!opt_invert) {
    //     char *where = NULL;
    //     while ( where == NULL && xgetline(bufp, bufszp, stream) > 0 ) {
    //         ++cnt_line;
    //
    //         where = (opt_ignore_case) ?
    //             strstr_ignore_case(*bufp, needle) : strstr(*bufp, needle);
    //     }
    //
    //     return where;
    // } else {
    //     char *where = needle;
    //     while (where != NULL && xgetline(bufp, bufszp, stream) > 0 ) {
    //         ++cnt_line;
    //
    //         where = (opt_ignore_case) ?
    //             strstr_ignore_case(*bufp, needle) : strstr(*bufp, needle);
    //     }
    //
    //     return (where == NULL) ? *bufp : NULL;
    // }

    return (!opt_invert) ?
        where :
        ((where != NULL) ? NULL : *bufp);
    // if (!opt_invert)
    //     return where;
    // else
    //     return (where != NULL) ? NULL : *bufp;
}


int cook_opts (int argc, char **argv) {
    int rval = 0;

    for (
            int opt = 0;
            rval == 0 && (opt = getopt(argc, argv, OPTS_SHORT)) != -1;
        ) {
        switch (opt) {

            case 'e':
                pattern = optarg;
                break;

            case 'i':
                opt_ignore_case = 1;
                break;

            case 'v':
                opt_invert = 1;
                break;

            case 'c':
                opt_count_only = 1;
                break;

            case 'l':
                opt_files_only = 1;
                break;

            case 'n':
                opt_number = 1;
                break;

            case 'h':
                opt_anonymous = 1;
                break;

            case 's':
                opt_silent = 1;
                break;

            // case 'f':
            //     break;

            case 'o':
                opt_matching_part = 1;
                break;

            default: // invalid option case
                rval = -1;
                usage();
                break;
        }
    }

#if DEBUG
    printf("[DEBUG] opt %-18s %c\n", "ignore case",        opt_ignore_case    ? '+' : '-');
    printf("[DEBUG] opt %-18s %c\n", "invert",             opt_invert         ? '+' : '-');
    printf("[DEBUG] opt %-18s %c\n", "count matches only", opt_count_only     ? '+' : '-');
    printf("[DEBUG] opt %-18s %c\n", "files only",         opt_files_only     ? '+' : '-');
    printf("[DEBUG] opt %-18s %c\n", "number",             opt_number         ? '+' : '-');
    printf("[DEBUG] opt %-18s %c\n", "anonymous",          opt_anonymous      ? '+' : '-');
    printf("[DEBUG] opt %-18s %c\n", "silent",             opt_silent         ? '+' : '-');
    printf("[DEBUG] opt %-18s %c\n", "matching part only", opt_matching_part  ? '+' : '-');
    printf("[DEBUG] search pattern: '%s'\n", pattern);
#endif

    return rval == -1 ? rval : optind;
}


void usage (void) {
    fputs("Usage: " PROGRAM_NAME ": [OPTION]... -e PATTERN [FILE]...\n", stdout);
    fputs("       " PROGRAM_NAME ": [OPTION]... -f PATTERN_FILE [FILE]...\n", stdout);
    fputs("Author: " AUTHOR "\n", stdout);

    return;
}


char *strstr_ignore_case (const char *str, const char *substr) {
    char *pos = NULL;

    for (char *strpos, *subpos; *str != '\0' && pos == NULL; ++str) {

        strpos = (char*)str;
        subpos = (char*)substr;
        while (*subpos != '\0' &&
               tolower((int) *strpos) == tolower((int) *subpos)) {
            ++strpos;
            ++subpos;
        }

        if (*subpos == '\0') {
            pos = (char*)str;
        }

    }

    return pos;
}

