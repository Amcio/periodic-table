CC=gcc
TARGET=periodic
OBJFILES=periodic.o tui.o
LIBS=-lncurses -lform -lmenu -lpanel
INCLUDES=-Iinclude
CFLAGS=

.PHONY: clean

$(TARGET): $(OBJFILES)
	if [ ! -d "bin" ]; then mkdir bin; fi
	$(CC) $(CFLAGS) -o bin/$@ $(OBJFILES) $(INCLUDES) $(LIBS)

periodic.o: periodic.c
	$(CC) -c -o $@ $^ $(CFLAGS) $(INCLUDES) $(LIBS)

tui.o: ncurses/tui.c
	$(CC) -c -o $@ $^ $(CFLAGS) $(INCLUDES) $(LIBS)

clean:
	rm bin/$(TARGET) *.o
