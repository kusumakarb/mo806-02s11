all: compressedfs.c compressedfs.h
	gcc compressedfs.c -o compressedfs -g -Wall `pkg-config fuse --cflags --libs`

clean:
	rm -f compressedfs
