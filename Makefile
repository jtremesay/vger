all: vger

clean:
	rm vger
	
vger: main.c
	${CC} -o vger main.c

test: vger
	cd tests && sh test.sh
