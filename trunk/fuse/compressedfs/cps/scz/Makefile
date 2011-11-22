include ../../Makefile.include

all: scz_main.c scz_main.h scz_core.c scz_core.h
	gcc scz_main.c scz_decompress_lib.c scz_compress_lib.c scz_core.c -c $(CF)

clean:
	rm -f *.o
