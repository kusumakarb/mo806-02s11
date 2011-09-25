all: babuino

babuino: babuinos.c desenho.c desenho.h
	gcc -g -m64 -lpthread -lncurses babuinos.c desenho.c -o babuinos

gdb: babuino
	gdb ./babuinos

run: babuino
	./babuinos

clean:
	rm -f babuinos
