PREFIX?=/usr/local/

all: vger

clean:
	rm -f vger *.core
	
vger: main.c
	${CC} -o vger main.c
	
install: vger
	install -o root -g wheel vger ${PREFIX}/bin/

test: vger
	cd tests && sh test.sh
