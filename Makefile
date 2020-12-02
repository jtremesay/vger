PREFIX?=/usr/local/

install: vger
	install -o root -g wheel vger ${PREFIX}/bin/

all: vger

clean:
	rm vger
	
vger: main.c
	${CC} -o vger main.c

test: vger
	cd tests && sh test.sh
