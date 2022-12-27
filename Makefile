CC=gcc
TARGET=periodic
LIBS=-lncurses
CFLAGS=

.PHONY: clean

periodic: periodic.c
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm $(TARGET)
