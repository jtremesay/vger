#include <err.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "utils.h"

void
eunveil(const char *path, const char *permissions)
{
	if (unveil(path, permissions) == -1) {
		syslog(LOG_DAEMON, "unveil on %s failed", path);
		err(1, "unveil on %s failed", path);
	}
}

void
epledge(const char *promises, const char *execpromises)
{
	if (pledge(promises, execpromises) == -1) {
		syslog(LOG_DAEMON, "pledge failed for: %s", promises);
		err(1, "pledge failed for: %s", promises);
	}
}

size_t
estrlcpy(char *dst, const char *src, size_t dstsize)
{
	size_t n = 0;

	n = strlcpy(dst, src, dstsize);
	if (n >= dstsize) {
        err(1, "strlcyp failed for %s = %s", dst, src);
	}

	return n;
}

size_t
estrlcat(char *dst, const char *src, size_t dstsize)
{
    size_t size;
    if ((size = strlcat(dst, src, dstsize)) >= dstsize)
        err(1, "strlcat on %s + %s", dst, src);

    return size;
}

int
esetenv(const char *name, const char *value, int overwrite)
{
	int ret = 0;
	ret = setenv(name, value, overwrite);

	if (ret != 0) {
		err(1, "setenv %s:%s", name, value);
	}

	return ret;
}

void
errlog(const char *format, ...)
{
	char e[1024] = {'\0'};
    va_list ap;

    va_start(ap, format);
    vsnprintf(e, sizeof(e), format, ap);
    va_end(ap);

	syslog(LOG_DAEMON, "%s", e);
	err(1, "%s", e);
}
