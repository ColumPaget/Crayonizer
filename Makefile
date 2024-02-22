CC = gcc
CFLAGS = -g -O2 -O2 -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 -fstack-protector-strong
LIBS = -lm -lz 
INSTALL=/usr/bin/install -c
prefix=/usr
bindir=$(prefix)${exec_prefix}/bin
DESTDIR=
LIBUSEFUL_BUNDLED=libUseful/libUseful.a
FLAGS=$(CFLAGS) -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE_URL=\"\" -DHAVE_STDIO_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_STRINGS_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_UNISTD_H=1 -DSTDC_HEADERS=1 -DHAVE_LIBZ=1



OBJ=common.o command_line.o config_file.o help.o crayonizations.o text_substitutions.o status_bar.o escape_sequences.o xterm.o keypress.o history.o signals.o timers.o

all: $(OBJ) main.c common.h $(LIBUSEFUL_BUNDLED)
	$(CC) $(FLAGS) -o crayonizer main.c $(LIBS) $(OBJ) $(LIBUSEFUL_BUNDLED)

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

$(LIBUSEFUL_BUNDLED):
	make -C libUseful

clean:
	@rm -f crayonizer *.o libUseful/*.o libUseful/*.a libUseful/*.so config.log config.status *.orig *~
	@rm -rf autom4te.cache 

install:
	-$(INSTALL) -d $(DESTDIR)$(bindir)
	-$(INSTALL) crayonizer $(DESTDIR)$(bindir)
	-$(INSTALL) -d $(DESTDIR)/etc/crayonizer.d
	-cp -n examples/* $(DESTDIR)/etc/crayonizer.d
	-$(INSTALL) -d $(DESTDIR)/$(prefix)/prebin
	-for PROG in ssh sftp tcpdump nmap make ifconfig ping ; do ln -s $(DESTDIR)$(bindir)/crayonizer $(DESTDIR)/$(prefix)/prebin/$$PROG; done




