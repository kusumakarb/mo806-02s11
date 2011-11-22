include Makefile.include

DUMMY = cps/dummy/dummy.o

SCZ = 	cps/scz/scz_compress_lib.o \
	cps/scz/scz_decompress_lib.o \
	cps/scz/scz_core.o \
	cps/scz/scz_main.o

LZO =   cps/lzo/minilzo.o \
	cps/lzo/lzo_main.o

COMPRESSED = compressedfs.o

CPS = $(DUMMY) $(SCZ) $(LZO)

OBJ = $(COMPRESSED) $(CPS)

all: cps compressed
	gcc $(OBJ) -o compressedfs $(CF)

cps:
	cd cps && $(MAKE)

compressed: compressedfs.c compressedfs.h
	gcc -c compressedfs.c $(CF)

clean:
	rm -f compressedfs $(COMPRESSED)
	cd cps && $(MAKE) clean

.PHONY: cps clean