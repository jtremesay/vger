PREFIX?=/usr/local/

all: vger

clean:
	rm -f vger *.core
	
vger: main.c
	${CC} -o vger main.c
	
install: vger
	install -o root -g wheel vger ${PREFIX}/bin/
	install -o root -g wheel vger.8 ${PREFIX}/man/man8/

test: vger
	cd tests && sh test.sh
