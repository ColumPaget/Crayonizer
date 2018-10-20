CC = gcc
CFLAGS = -g -O2
LIBS = -lm -lz 
INSTALL=/bin/install -c
prefix=/usr/local
bindir=$(prefix)${exec_prefix}/bin
DESTDIR=
FLAGS=$(CFLAGS) -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE_URL=\"\" -DSTDC_HEADERS=1 -DHAVE_LIBZ=1



OBJ=common.o command_line.o config_file.o help.o crayonizations.o text_substitutions.o status_bar.o escape_sequences.o xterm.o keypress.o history.o signals.o timers.o

all: $(OBJ) main.c common.h
	@cd libUseful; $(MAKE)
	$(CC) $(FLAGS) -o crayonizer main.c $(LIBS) $(OBJ) libUseful/libUseful.a 

config_file.o: config_file.c config_file.h common.h
	$(CC) $(FLAGS) -c config_file.c

command_line.o: command_line.c command_line.h common.h
	$(CC) $(FLAGS) -c command_line.c

common.o: common.c common.h common.h
	$(CC) $(FLAGS) -c common.c

crayonizations.o: crayonizations.c crayonizations.h common.h
	$(CC) $(FLAGS) -c crayonizations.c

help.o: help.c help.h common.h
	$(CC) $(FLAGS) -c help.c

xterm.o: xterm.c xterm.h common.h
	$(CC) $(FLAGS) -c xterm.c

keypress.o: keypress.c keypress.h common.h
	$(CC) $(FLAGS) -c keypress.c

history.o: history.c history.h common.h
	$(CC) $(FLAGS) -c history.c

signals.o: signals.c signals.h common.h
	$(CC) $(FLAGS) -c signals.c

text_substitutions.o: text_substitutions.c text_substitutions.h common.h
	$(CC) $(FLAGS) -c text_substitutions.c

escape_sequences.o: escape_sequences.c escape_sequences.h common.h
	$(CC) $(FLAGS) -c escape_sequences.c

status_bar.o: status_bar.c status_bar.h common.h
	$(CC) $(FLAGS) -c status_bar.c

timers.o: timers.c timers.h common.h
	$(CC) $(FLAGS) -c timers.c


clean:
	@rm -f crayonizer *.o libUseful/*.o libUseful/*.a libUseful/*.so config.log config.status
	@rm -rf autom4te.cache 

install:
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL) crayonizer $(DESTDIR)$(bindir)


