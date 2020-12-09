PREFIX?=/usr/local/
CFLAGS  += -pedantic -Wall -Wextra -Wmissing-prototypes \
           -Wstrict-prototypes -Wwrite-strings

all: vger

clean:
	rm -f vger *.core *.o

vger: main.o mimes.o
	${CC} -o vger main.o mimes.o

install: vger
	install -o root -g wheel vger ${PREFIX}/bin/
	install -o root -g wheel vger.8 ${PREFIX}/man/man8/

test: vger
	cd tests && sh test.sh
