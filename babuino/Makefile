all: babuino

babuino: babuinos.c desenho.c desenho.h
	gcc -Wall -g -lpthread -lncurses babuinos.c desenho.c -o babuinos

debug:
	gcc -Wall -g -lpthread -lncurses babuinos.c desenho.c -o babuinos -DDEBUG

gdb: babuino
	gdb ./babuinos

run: babuinos
	./babuinos

clean:
	rm -f babuinos
