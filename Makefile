
all: mmage.so testmmage

mmage.o: mmage.c
	gcc -c -g -O3 -Wall `sdl2-config --cflags` $<

mmage.so: mmage.o
	gcc -rdynamic -g -bundle -undefined dynamic_lookup -o $@ $< `sdl2-config --libs` -lSDL2_image

testmmage: Mmage.uo testmmage.sml
	mosmlc testmmage.sml -o $@

Mmage.uo: Mmage.ui Mmage.sml
	mosmlc -c Mmage.sml

Mmage.ui: Mmage.sig
	mosmlc -c Mmage.sig
