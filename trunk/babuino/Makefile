all: babuino

babuino: babuinos.c
	gcc -m64 -O3 -lpthread babuinos.c -o babuinos

run: babuino
	./babuinos

clean:
	rm -f babuinos