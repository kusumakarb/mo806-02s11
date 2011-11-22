include ../../Makefile.include

SOURCES = minilzo.c lzo_main.c

#
# gcc (generic)
#

all: $(SOURCES)
	gcc -c $(SOURCES) $(CF)

#
# other targets
#

clean:
	rm -f *.o

.PHONY: default clean

