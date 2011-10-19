all: ramfs.h ramfs.c log.c opt.c
	gcc log.c ramfs.c opt.c -o ramfs -g -Wall `pkg-config fuse --cflags --libs`

clean:
	rm -f ramfs
	-fusermount -u test

prepare: all
	mkdir -p test
	-fusermount -u test
test: prepare
	./ramfs -s test

gdb: test
#	gdb --args ramfs test
	sudo gdb ramfs `ps aux | grep ./ramfs | cut -b11-15 | head -n1`