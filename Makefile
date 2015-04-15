CC = gcc
VERSION = 0.0.3
CFLAGS = -g -O2
LIBS = -lm -lz 
INSTALL=/bin/install -c
prefix=/usr/local
bindir=$(prefix)${exec_prefix}/bin
DESTDIR=
FLAGS=$(CFLAGS) -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DSTDC_HEADERS=1 -DHAVE_LIBZ=1



OBJ=common.o config_file.o help.o crayonizations.o status_bar.o xterm.o keypress.o

all: $(OBJ) main.c common.h
	@cd libUseful-2.1; $(MAKE)
	$(CC) $(FLAGS) -o crayonizer main.c $(LIBS) $(OBJ) libUseful-2.1/libUseful-2.1.a 

config_file.o: config_file.c config_file.h common.h
	gcc -c config_file.c

common.o: common.c common.h common.h
	gcc -c common.c

crayonizations.o: crayonizations.c crayonizations.h common.h
	gcc -c crayonizations.c

help.o: help.c help.h common.h
	gcc -c help.c

xterm.o: xterm.c xterm.h common.h
	gcc -c xterm.c

keypress.o: keypress.c keypress.h common.h
	gcc -c keypress.c

status_bar.o: status_bar.c status_bar.h common.h
	gcc -g -c status_bar.c

clean:
	@rm -f crayonizer *.o libUseful-2.1/*.o libUseful-2.1/*.a libUseful-2.1/*.so

install:
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL) crayonizer $(DESTDIR)$(bindir)


