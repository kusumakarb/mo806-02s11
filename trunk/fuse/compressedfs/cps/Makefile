all: scz dummy lzo


dummy:
	cd dummy && $(MAKE)

scz:
	cd scz && $(MAKE)

lzo:
	cd lzo && $(MAKE)

clean:
	cd dummy && $(MAKE) clean
	cd scz && $(MAKE) clean
	cd lzo && $(MAKE) clean

.PHONY: dummy scz clean lzo