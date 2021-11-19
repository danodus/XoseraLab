# Makefile
# vim: set noet ts=8 sw=8

LDFLAGS		:= $(shell sdl2-config --libs) -lm
SDL_CFLAGS	:= $(shell sdl2-config --cflags)

CFLAGS		:= -g -std=c99 $(SDL_CFLAGS)

all: racing_sdl

racing_sdl: racing_sdl.c racing.c
	$(CC) $(CFLAGS) racing_sdl.c racing.c -o racing_sdl $(LDFLAGS) 

clean:
	rm -f racing_sdl

.PHONY: all clean
