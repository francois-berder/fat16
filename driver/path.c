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

int fat16_get_short_filename(char *filename, const char *path)
{
    uint8_t i = 0;
    uint8_t sep;

    if (path[0] == '/')
        ++path;

    /* Find position of . (marker between name and extension) */
    for (i = 0; i < 9; ++i) {
        if (path[i] == '\0')
            return -1;
        if (path[i] == '.') {
            sep = i;
            break;
        }

        if (!is_character_valid(path[i])) {
            FAT16DBG("FAT16: Invalid character in path: %s\n", path);
            return -1;
        }
    }

    /* If it cannot find . in the first 9 characters then the name is more
     * than 8 characters long which is forbidden. */
    if (i == 9)
        return -1;

    /* Copy name */
    for (i = 0; i < sep; ++i)
        filename[i] = toupper(path[i]);

    memset(&filename[sep], ' ', 8 - sep);

    /* Copy extension */
    for (i = 0; i < 3; ++i) {
        if (path[i] == '\0')
            return -1;

        if (!is_character_valid(path[sep + 1 + i])) {
            FAT16DBG("FAT16: Invalid character in path: %s\n", path);
            return -1;
        }

        filename[8 + i] = toupper(path[sep + 1 + i]);
    }

    return 0;
}
