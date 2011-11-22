all: scz dummy

.PHONY: dummy
dummy:
	cd dummy && $(MAKE)

.PHONY: scz
scz:
	cd scz && $(MAKE)

clean:
	cd dummy && $(MAKE) clean
	cd scz && $(MAKE) clean