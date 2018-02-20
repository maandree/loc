.POSIX:

CONFIGFILE = config.mk
include $(CONFIGFILE)

all: loc
loc.o: loc.c arg.h

loc: loc.o
	$(CC) -o $@ loc.o $(LDFLAGS)

check: loc
	test "$$(./loc < tests/0)" = 0
	test "$$(./loc - < tests/0)" = 0
	test "$$(./loc tests/1 tests/2 | sed 's/:  */: /')" = "$$(printf '%s\n' 'tests/1: 0' 'tests/2: 1' 1)"
	test "$$(./loc -s tests/1 tests/2)" = 1
	test "$$(./loc tests/0)" = 0
	test "$$(./loc tests/1)" = 0
	test "$$(./loc tests/2)" = 1
	test "$$(./loc tests/3)" = 0
	test "$$(./loc tests/4)" = 2
	test "$$(./loc tests/5)" = 2
	test "$$(./loc tests/6)" = 1
	test "$$(./loc tests/7)" = 0
	test "$$(./loc tests/8)" = 1
	test "$$(./loc tests/9)" = 1
	test "$$(./loc tests/10)" = 2
	test "$$(./loc tests/11)" = 0
	test "$$(./loc tests/12)" = 1
	test "$$(./loc tests/13)" = 2
	test "$$(./loc tests/14)" = 1
	test "$$(./loc tests/15)" = 0
	test "$$(./loc tests/16)" = 0
	test "$$(./loc tests/17)" = 1
	test "$$(./loc tests/18)" = 0
	test "$$(./loc tests/19)" = 1
	test "$$(./loc tests/20)" = 1
	test "$$(./loc tests/21)" = 1
	test "$$(./loc tests/22)" = 1
	test "$$(./loc tests/23)" = 1
	test "$$(./loc tests/24)" = 1
	test "$$(./loc tests/25)" = 0
	test "$$(./loc tests/26)" = 3
	test "$$(./loc tests/27)" = 3
	test "$$(./loc tests/28)" = 3
	test "$$(./loc tests/29)" = 3
	test "$$(./loc tests/30)" = 1
	test "$$(./loc tests/31)" = 1
	test "$$(./loc tests/32)" = 1
	test "$$(./loc tests/33)" = 1
	test "$$(./loc tests/34)" = 2
	test "$$(./loc tests/35)" = 2
	test "$$(./loc tests/36)" = 2
	test "$$(./loc tests/37)" = 1
	test "$$(./loc tests/38)" = 0
	test "$$(./loc tests/39)" = 1
	test "$$(./loc tests/40)" = 2

install: loc
	mkdir -p -- "$(DESTDIR)$(PREFIX)/bin/"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man1/"
	cp -- loc "$(DESTDIR)$(PREFIX)/bin/loc"
	cp -- loc.1 "$(DESTDIR)$(MANPREFIX)/man1/loc.1"

uninstall:
	-rm -f -- "$(DESTDIR)$(PREFIX)/bin/loc"
	-rm -f -- "$(DESTDIR)$(MANPREFIX)/man1/loc.1"

clean:
	-rm -f -- loc.o loc

.SUFFIXES:
.SUFFIXES: .o .c

.PHONY: all check install uninstall clean
