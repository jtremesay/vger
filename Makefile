PREFIX?=/usr/local/
CFLAGS  += -pedantic -Wall -Wextra -Wmissing-prototypes \
           -Wstrict-prototypes -Wwrite-strings

.SUFFIXES: .c .o

.c.o:
	${CC} ${CFLAGS} -c $<

all: vger

clean:
	find . -name vger -o \
		-name \*\.o -o \
		-name \*\.core \
		-delete

vger: main.o mimes.o utils.o opts.h
	${CC} ${CFLAGS} -o $@ main.o mimes.o utils.o

install: vger
	install -o root -g wheel vger ${PREFIX}/bin/
	install -o root -g wheel vger.8 ${PREFIX}/man/man8/

test: vger
	cd tests && sh test.sh
