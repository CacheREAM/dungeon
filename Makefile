CC=gcc
CFLAGS=-Wall -Wextra
LDFLAGS=-lncurses -lm

dungeon: main.c
	$(CC) $(CFLAGS) main.c -o dungeon $(LDFLAGS)

clean:
	rm -f dungeon
