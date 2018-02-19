.POSIX:

CONFIGFILE = config.mk
include $(CONFIGFILE)

all: loc
loc.o: loc.c arg.h

loc: loc.o
	$(CC) -o $@ loc.o $(LDFLAGS)

check:
	true

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
