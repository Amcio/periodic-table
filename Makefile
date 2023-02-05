CC=gcc
TARGET=periodic
OBJFILES=obj/periodic.o obj/tui.o
LIBS=-lncurses -lform -lmenu -lpanel
INCLUDES=-Iinclude
CFLAGS=

.PHONY: clean

$(TARGET): $(OBJFILES) | bin
	$(CC) $(CFLAGS) $(OBJFILES) $(LIBS) -o bin/$@

obj/%.o: src/%.c | obj
	$(CC) $(INCLUDES) $(CFLAGS) -c -o $@ $<

bin obj:
	@mkdir -p $@

clean:
	@$(RM) -rv bin obj
