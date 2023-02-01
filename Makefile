CC=gcc
TARGET=periodic
LIBS=-lncurses -lform -lmenu -lpanel
INCLUDES=-Iinclude
CFLAGS=

.PHONY: clean

tui: ncurses/tui.c
	if [ ! -d "bin" ]; then mkdir bin; fi
	$(CC) -o bin/$@ $^ $(CFLAGS) ${INCLUDES} $(LIBS)

periodic: periodic.c
	if [ ! -d "bin" ]; then mkdir bin; fi
	$(CC) -o bin/$@ $^ $(CFLAGS) ${INCLUDES} $(LIBS)

clean:
	rm bin/$(TARGET)
