OBJS = main.o proc.o check.o user.o log.o wlist.o

CC = gcc
CFLAGS = -Wall -Wextra -O2 -ggdb

all: ninja

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

ninja: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o ninja

