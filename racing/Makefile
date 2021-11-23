# Makefile
# vim: set noet ts=8 sw=8

all: sdl xosera

sdl:
	$(MAKE) -f sdl.mk

xosera:
	$(MAKE) -f xosera.mk

clean:
	$(MAKE) -f sdl.mk clean
	$(MAKE) -f xosera.mk clean

.PHONY: all clean sdl xosera
