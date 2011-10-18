all: ramfs.c
	gcc ramfs.c -o ramfs -Wall `pkg-config fuse --cflags --libs`

test: all
	mkdir -p test
	-fusermount -u test
	./ramfs test

clean:
	rm -f ramfs