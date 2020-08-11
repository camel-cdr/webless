
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man
LIBDIR = $(PREFIX)/lib/webless

webless: webless.c
	${CC} $< -o $@ `pkg-config --cflags --libs webkit2gtk-4.0` -Wall -Wextra -O3

run: webless
	./webless

install: webless
	cp -f webless /usr/local/bin/webless
	chmod 755 /usr/local/bin/webless

uninstall:
	rm -rf /usr/local/bin/webless

clean:
	rm -f webless

.PHONY: install uninstall clean
