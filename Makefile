CC=gcc
CFLAGS= -Iinclude -Wall -g
LDFLAGS= -Wl,-rpath=/usr/local/lib


.PHONY: clean

SRC = $(wildcard ./src/*.c)
OBJS = $(patsubst %.c, %.o, $(SRC))

./src/%.o: ./src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@ 

ttorrent: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^ -ldl

clean:
	rm -f $(OBJS) ttorrent
