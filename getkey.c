#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

int main(void) {
    int ch;
    initscr();
    keypad(stdscr, TRUE);
    noecho();
    ch = getch();
    endwin();
    printf("%d", ch);

}
