all: compressedfs.c compressedfs.h cps_dummy.c cps_dummy.h
	gcc compressedfs.c cps_dummy.c -o compressedfs -g -Wall `pkg-config fuse --cflags --libs`

clean:
	rm -f compressedfs
