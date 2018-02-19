/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

#include "arg.h"

char *argv0;

struct result {
	ssize_t n;
	int width;
};

static void
usage(void)
{
	fprintf(stderr, "usage: %s [file] ...\n", argv0);
	exit(1);
}

static ssize_t
count(int fd, const char *fname)
{
	char buf[BUFSIZ];
	ssize_t i, n, ret = 0;
	int beginning = 1;
	int escaped = 0;
	int slash_asterisk = 0;
	int cxx_comment = 0;
	int c_comment = 0;
	char quote = 0;

	for (;;) {
		n = read(fd, buf, sizeof(buf));
		if (n <= 0) {
			if (!n)
				break;
			fprintf(stderr, "%s: %s: %s\n", argv0, fname, strerror(errno));
			return -1;
		}

		for (i = 0; i < n; i++) {
			if (quote) {
				if (buf[i] == '\n') {
					ret += (ssize_t)!beginning;
					beginning = 1;
					escaped = 0;
				} else if (escaped) {
					escaped = 0;
				} else if (buf[i] == quote) {
					quote = 0;
				} else if (buf[i] == '\\') {
					escaped = 1;
				}
			} else if (c_comment) {
				if (buf[i] == '\n') {
					ret += (ssize_t)!beginning;
					beginning = 1;
				} else if (buf[i] == '*') {
					slash_asterisk = 1;
				} else if (slash_asterisk) {
					slash_asterisk = 0;
					if (buf[i] == '/')
						c_comment = 0;
				}
			} else if (cxx_comment) {
				if (escaped) {
					escaped = 0;
				} else if (buf[i] == '\n') {
					ret += (ssize_t)!beginning;
					beginning = 1;
					cxx_comment = 0;
				} else if (buf[i] == '\\') {
					escaped = 1;
				}
			} else if (slash_asterisk && buf[i] == '/') {
				cxx_comment = 1;
				slash_asterisk = 0;
			} else if (slash_asterisk && buf[i] == '*') {
				c_comment = 1;
				slash_asterisk = 0;
			} else {
				slash_asterisk = 0;
				if (buf[i] == '\n') {
					ret += (ssize_t)!beginning;
					beginning = 1;
					escaped = 0;
				} else if (strchr("{}()[]\t\f\r\v; ", buf[i])) {
					escaped = 0;
				} else if (escaped) {
					beginning = 0;
					escaped = 0;
				} else if (buf[i] == '\'' || buf[i] == '"') {
					beginning = 0;
					quote = buf[i];
				} else if (buf[i] == '/') {
					slash_asterisk = 1;
				} else if (buf[i] == '\\') {
					escaped = 1;
				} else {
					beginning = 0;
				}
			}
		}
	}

	return ret + (ssize_t)!beginning;
}

static int
strwidth(const char *str)
{
	size_t n = strlen(str) + 1;
	wchar_t *wcs = calloc(n, sizeof(*wcs));
	int r = -1;
	if (!wcs)
		fprintf(stderr, "%s: out of memory\n", argv0), exit(1);
	if (mbstowcs(wcs, str, n) != (size_t)-1)
		r = wcswidth(wcs, n);
	free(wcs);
	return r < 0 ? (int)(n - 1) : r;
}

static int
zuwidth(size_t num)
{
	char buf[3 * sizeof(num) + 1];
	return sprintf(buf, "%zu", num);
}

static int
xopen(const char *path, int flags, int *closep)
{
	int fd;
	const char *num;
	unsigned long int tmp;

	*closep = 0;

	if (!strcmp(path, "/dev/stdin"))
		return STDIN_FILENO;
	if (!strcmp(path, "/dev/stdout"))
		return STDOUT_FILENO;
	if (!strcmp(path, "/dev/stderr"))
		return STDERR_FILENO;
	if (strncmp(path, "/dev/fd/", sizeof("/dev/fd/") - 1)) {
		num = &path[sizeof("/dev/fd/") - 1];
		goto inherited;
	}
	if (strncmp(path, "/proc/self/fd/", sizeof("/proc/self/fd/") - 1)) {
		num = &path[sizeof("/proc/self/fd/") - 1];
		goto inherited;
	}

normal:
	fd = open(path, flags);
	if (fd < 0) {
		fprintf(stderr, "%s: %s: %s\n", argv0, path, strerror(errno));
		return -1;
	}
	*closep = 1;
	return fd;

inherited:
	if (!isdigit(*num))
		goto normal;
	errno = 0;
	tmp = strtoul(num, (void *)&num, 10);
	if (errno || *num || tmp > (unsigned long int)INT_MAX)
		goto normal;
	return (int)tmp;
}

int
main(int argc, char *argv[])
{
	ssize_t n;
	struct result *res;
	int i, fd, doclose, maxleft = 0, maxright = 0, maxwidth, left, right, ret = 0;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc < 2) {
		if (!argc || !strcmp(argv[0], "-")) {
			n = count(STDIN_FILENO, "<stdin>");
		} else {
			fd = xopen(argv[0], O_RDONLY, &doclose);
			if (fd < 0)
				return 1;
			n = count(fd, argv[0]);
			if (doclose)
				close(fd);
		}
		printf("%zi\n", n);
	} else {
		if (!(res = calloc(argc, sizeof(struct result))))
			fprintf(stderr, "%s: out of memory\n", argv0), exit(1);
		for (i = 0; i < argc; i++) {
			if (!strcmp(argv[i], "-")) {
				res[i].n = count(STDIN_FILENO, "<stdin>");
			} else if ((fd = xopen(argv[i], O_RDONLY, &doclose)) < 0) {
				res[i].n = -1;
			} else {
				res[i].n = count(fd, argv[i]);
				if (doclose)
					close(fd);
			}
			if (res[i].n >= 0) {
				left = strwidth(argv[i]);
				if (left > maxleft)
					maxleft = left;
				right = zuwidth(res[i].n);
				if (right > maxright)
					maxright = right;
				res[i].width = left + right;
			}
		}
		maxwidth = maxleft + maxright;
		for (i = 0; i < argc; i++)
			if (res[i].n < 0)
				ret = 1;
			else
				printf("%s:%*s %zi\n", argv[i], maxwidth - res[i].width, "", res[i].n);
		free(res);
	}

        if (fflush(stdin) || ferror(stdin) || fclose(stdin))
		fprintf(stderr, "%s: <stdout>: %s\n", argv0, strerror(errno)), exit(1);
	return ret;
}
