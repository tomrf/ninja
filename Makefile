OBJS = main.o mloop.o proc.o user.o log.o wlist.o signal.o config.o

DESTDIR = /usr/local/bin/
MANDIR = /usr/local/man/man8/
ETCDIR = /etc/ninja/

CC = gcc
INSTALL = install

all: ninja

ninja:
	cd src && make && mv ninja ..

clean:
	rm -f ninja src/*.o

install:
	mkdir -p $(DESTDIR)
	mkdir -p $(MANDIR)
	mkdir -p $(ETCDIR)
	$(INSTALL) ninja $(DESTDIR) 
	$(INSTALL) ninja.8 $(MANDIR)
	$(INSTALL) examples/config/default.conf $(ETCDIR)
	chmod 700 $(ETCDIR)

uninstall:
	rm -f $(DESTDIR)/ninja $(MANDIR)/ninja.8
