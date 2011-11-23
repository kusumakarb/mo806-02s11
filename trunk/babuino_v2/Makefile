all: babuino

babuino: babuinos.c
	gcc -Wall -g -lpthread  babuinos.c -o babuinos

gdb: babuino
	gdb ./babuinos

run: babuino
	./babuinos

clean:
	rm -f babuinos
