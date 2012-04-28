#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "common.h"

/* log levels:
 *    0 - alerts, errors
 *    1 - warnings
 *    2 - info
 *    3 - verbose
 *    4 - debug
 */

void LOG(const int level, char *fmt, ...)
{
    va_list     args;

    fprintf(stdout, "LOG (%d): ", level);

    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);

    puts("");
}
