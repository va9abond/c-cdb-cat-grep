/* cat -- concatenate files and print on the standard output.

   Always unbuffered, -u is ignored.

Refs:
    https://github.com/pete/cats
        freebsd-cat.c, openbsd-cat.c, gnu-cat.c, aix-cat.c
    https://github.com/coreutils/coreutils/blob/master/src/cat.c
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>


#define DEBUG 0
#define PROGRAM_NAME "cat"
#define AUTHOR "Rustem Sirazetdinov"


static char const *const OPTS_SHORT = "benstuvAETh";

static struct option const OPTS_LONG[] =
{
    { "number",           no_argument, NULL, 'n' },
    { "number-nonblank",  no_argument, NULL, 'b' },
    { "show-nonprinting", no_argument, NULL, 'v' },
    { "squeeze-blank",    no_argument, NULL, 's' },
    { "show-tabs",        no_argument, NULL, 'T' },
    { "show-ends",        no_argument, NULL, 'E' },
    { "show-all",         no_argument, NULL, 'A' },
    { "help",             no_argument, NULL, 'h' },
    { NULL, 0, NULL, 0 }
};

static char opt_number           = 0;
static char opt_number_nonblank  = 0;
static char opt_show_nonprinting = 0;
static char opt_show_tabs        = 0;
static char opt_show_ends        = 0;
static char opt_squeeze_blank    = 0;

static void usage (void);
static void usage_brief (void);

static size_t cnt_line = 0;
static int cat (FILE *istream, FILE *ostream);


int main (int argc, char *argv[]) {
    int rval = 0;

    // Cook options
    for (
            int opt = 0;
            rval == 0 &&
              (opt = getopt_long(argc, argv, OPTS_SHORT, OPTS_LONG, NULL)) != -1;
        ) {
        switch (opt) {
            case 'b':
                opt_number = 1;
                opt_number_nonblank = 1;
                break;

            case 'e':
                opt_show_ends = 1;
                opt_show_nonprinting = 1;
                break;

            case 'n':
                opt_number = 1;
                break;

            case 's':
                opt_squeeze_blank = 1;
                break;

            case 't':
                opt_show_tabs = 1;
                opt_show_nonprinting = 1;
                break;

            case 'u':
                // We provide the -u feature unconditionally.
                break;

            case 'v':
                opt_show_nonprinting = 1;
                break;

            case 'A':
                opt_show_nonprinting = 1;
                opt_show_ends = 1;
                opt_show_tabs = 1;
                break;

            case 'E':
                opt_show_ends = 1;
                break;

            case 'T':
                opt_show_tabs = 1;
                break;

            case 'h':
                usage();
                rval = 1;
                break;

            default: // invalid option case
                rval = -1;
                usage_brief();
                break;
        }
    }

#if DEBUG
    printf("[DEBUG] opt %-21s %c\n", "number (-n)",           opt_number           ? '+' : '-');
    printf("[DEBUG] opt %-21s %c\n", "number_nonblank (-b)",  opt_number_nonblank  ? '+' : '-');
    printf("[DEBUG] opt %-21s %c\n", "show_nonprinting (-v)", opt_show_nonprinting ? '+' : '-');
    printf("[DEBUG] opt %-21s %c\n", "show_tabs (-T)",        opt_show_tabs        ? '+' : '-');
    printf("[DEBUG] opt %-21s %c\n", "show_ends (-E)",        opt_show_ends        ? '+' : '-');
    printf("[DEBUG] opt %-21s %c\n", "squeeze_blank (-s)",    opt_squeeze_blank    ? '+' : '-');
#endif

    FILE *istream = NULL;
    FILE *ostream = stdout;

    // Does we should read from STDIN?
    char flag_stdin = 0;
    if (optind == argc) {
        --optind;
        flag_stdin = 1;
    }

    // Main loop
    for (
            char **argp = argv + optind;
            optind < argc && rval == 0;
            ++optind, ++argp
    ) {
        if ( flag_stdin || ((*argp)[0] == '-' && (*argp)[1] == '\0' ) )
            istream = stdin;
        else
            istream = fopen(*argp, "r");

        if (istream == NULL && (rval = -1)) {
            printf("[ERROR] " PROGRAM_NAME ": cannot open file: %s\n", *argp);
            break;
        }

        rval = cat(istream, ostream);

        if (istream != stdin)
            (void) fclose(istream);
        else {
            clearerr(stdin); // clear feof(stdin) flag
            (void) fflush(ostream);
        }
    }

    return (rval == 0 || rval == 1) ? 0 : rval;
}


int cat (FILE *istream, FILE *ostream) {
    int rval = 0;

    // Main cat loop
    for (int ch = fgetc(istream), eol = 0;
            ch != EOF && !ferror(istream) && !ferror(ostream);
    ) {
        while (ch == '\n') {
            ++eol;

            // Did the last line empty
            if (eol > 0) {
                // Were there more then 2 consecutive empty liles
                if (eol >= 2) {
                    eol = 2;

                    // Squezee lines? (-s)
                    if (opt_squeeze_blank) {
                        ch = fgetc(istream);
                        continue;
                    }
                }

                // Number empty lines? (-n, but without -b)
                if (opt_number && !opt_number_nonblank)
                    (void) fprintf(ostream, "%6lu\t", ++cnt_line);
            }

            // Show End Of Line symbol? (-e)
            if (opt_show_ends)
                (void) fputc('$', ostream);

            (void) fputc('\n', ostream);
            ch = fgetc(istream);
        }


        // Are we at the beginning of a line, and line numbers are requested?
        if (opt_number && ch != EOF)
            (void) fprintf(ostream, "%6lu\t", ++cnt_line);


        // Here 'ch' cannot contain a newline character.

        // If at least one of (-v), (-e), (-t) specified
        if (opt_show_nonprinting) {

            while (ch != '\n' && ch != EOF) {
                if (ch < 32) { // control chars

                    if (ch == '\t' && !opt_show_tabs)
                        (void) fputc('\t', ostream);
                    else {
                        (void) fputc('^', ostream);
                        (void) fputc(ch + 64, ostream);
                    }

                } else if (ch < 127) { // isprint(ch), ch in [32, 127)
                    (void) fputc(ch, ostream);
                } else if (ch == 127) { // DEL, iscntrl(ch)
                    (void) fputc('^', ostream);
                    (void) fputc('?', ostream);
                } else { // extended ASCII codes [128, 255]
                    (void) fputc('M', ostream);
                    (void) fputc('-', ostream);

                    if (ch < 128 + 32) { // [128, 160)
                        (void) fputc('^', ostream);
                        (void) fputc(ch - 128 + 64, ostream);
                    } else if (ch < 128 + 127) { // [160, 255)
                        (void) fputc(ch - 128, ostream);
                    } else { // [255, ...)
                        (void) fputc('^', ostream);
                        (void) fputc('?', ostream);
                    }
                }

                ch = fgetc(istream);
            }

        } else { // (not quoting)

            while (ch != '\n' && ch != EOF) {
                if (ch == '\t' && opt_show_tabs) {
                    (void) fputc('^', ostream);
                    (void) fputc(ch + 64, ostream);
                } else {
                    (void) fputc(ch, ostream);
                }

                ch = fgetc(istream);
            }

        } // assert(ch == '\n' || ch == EOF);

        eol = -1;
    }

    if (ferror(istream) != 0 && (rval = -1))
        printf("[ERROR] " PROGRAM_NAME ": error occured while reading input stream\n");

    if (ferror(ostream) != 0 && (rval = -1))
        printf("[ERROR] " PROGRAM_NAME ": error occured while writing in output stream\n");

    if (feof(istream) == 0)
        rval = -1;
#if DEBUG
    else
        printf("[DEBUG] end of input stream\n");
#endif

    return rval;
}


void usage (void) {
    (void) fputs ("\
            Usage: " PROGRAM_NAME " [OPTION]... [FILE]...\n\
            Concatenate FILE(s), or standard input, to standard output.\n\
            \n\
            -A, --show-all           equivalent to -vET\n\
            -b, --number-nonblank    number nonempty output lines\n\
            -e                       equivalent to -vE\n\
            -E, --show-ends          display $ at end of each line\n\
            -n, --number             number all output lines\n\
            -s, --squeeze-blank      suppress repeated empty output lines\n\
            -t                       equivalent to -vT\n\
            -T, --show-tabs          display TAB characters as ^I\n\
            -u                       (ignored)\n\
            -v, --show-nonprinting   use ^ and M- notation, except for LFD and TAB\n\
            -h, --help               display this help and exit\n\
            \n\
            Examples:\n\
            cat f - g  Output f's contents, then standard input, then g's contents.\n\
            cat        Copy standard input to standard output.\n\
            \n\
            Author: " AUTHOR "\n\
            ", stdout);

    return;
}


void usage_brief (void) {
    (void) fputs("Try '" PROGRAM_NAME " --help' for more information.\n", stdout);

    return;
}


// TODO check stdout is valid
// if (fstat((int)fileno(stdout), &statb) < 0) {
//     if (!silent)
//         fprintf(stderr, MSGSTR(ESTATOUT,"cat: cannot stat stdout\n"));
//     exit(2);
// }

// TODO make rval global error variable
// TODO move options parsing in separate function, because it don't deal with
//      any local for main variables except rval uless we make rval global
// TODO SEND_INFO, SEND_ERROR, SEND_DEBUG
// TODO line_number -> line_buf like in GNU
// TODO fast cat for simple options. Low level copying from input stream

// task write simple cat program
//      + [x] cat from stdin
//      + [x] cat from 1 file
//      + [x] cat from stdin or from 1 file
//      + [x] cat from multiple files
//      + [x] parse short opts
//      + [x] parse long opts
//      + [x] apply opts
