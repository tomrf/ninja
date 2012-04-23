OBJS = main.o mloop.o proc.o user.o log.o wlist.o signal.o config.o file.o

CC = gcc
CFLAGS = -Wall -O2

all: ninja

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

ninja: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o ninja

