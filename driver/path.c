#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "path.h"

static bool is_character_valid(char c)
{
    return ('A' <= c && c <= 'Z')
           || ('0' <= c && c <= '9')
           || c == '!'
           || c == '#'
           || c == '$'
           || c == '%'
           || c == '&'
           || c == '\''
           || c == '('
           || c == ')'
           || c == '-'
           || c == '@'
           || c == '^'
           || c == '_'
           || c == '`'
           || c == '{'
           || c == '}'
           || c == '~';
}

int to_short_filename(char *short_filename, const char *long_filename)
{
    uint8_t i = 0;
    uint8_t sep;

    if (long_filename[0] == '/')
        ++long_filename;

    if (long_filename[0] == '\0')
        return -1;

    /* Find position of . (marker between name and extension) */
    for (i = 0; i < 9; ++i) {
        /* If that is already the end of file, fill with spaces and exit */
        if (long_filename[i] == '\0') {
            memset(&short_filename[i+1], ' ', 11 - i);
            return 0;
        }

        if (long_filename[i] == '.') {
            sep = i;
            break;
        }

        if (!is_character_valid(long_filename[i])) {
            return -1;
        }

        short_filename[i] = long_filename[i];
    }

    /*
     * If it cannot find . in the first 9 characters then the name is more
     * than 8 characters long which is forbidden.
     */
    if (i == 9)
        return -1;

    memset(&short_filename[sep], ' ', 8 - sep);

    /* Copy extension */
    for (i = 0; i < 3; ++i) {
        if (long_filename[sep + 1 + i] == '\0') {
            break;
        }

        if (!is_character_valid(long_filename[sep + 1 + i])) {
            return -1;
        }

        short_filename[8 + i] = long_filename[sep + 1 + i];
    }
    /* Check that extension consists of no more than 3 characters */
    if (i == 3 && long_filename[sep + 4] != '\0')
        return -1;

    memset(&short_filename[8+i], ' ', 3-i);

    return 0;
}
