CC=gcc
TARGET=periodic
LIBS=-lncurses -lform
CFLAGS=

.PHONY: clean

periodic: periodic.c
	if [ ! -d "bin" ]; then mkdir bin; fi
	$(CC) -o bin/$@ $^ $(CFLAGS) $(LIBS)

clean:
	rm bin/$(TARGET)
