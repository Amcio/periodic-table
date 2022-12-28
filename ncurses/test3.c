#include <menu.h>

int main(void) {
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    // init_pair(1, COLOR_BLACK, COLOR_WHITE);

    attron(A_REVERSE);
    mvprintw(LINES - 1, 0, "F1 Add F2 Remove F3 Search F10 Exit");
    attroff(A_REVERSE);

    int split = (0.75 * LINES) - 1; // Point of window split in 2/3 of the screen
    WINDOW* menu_win = newwin(split, COLS, 0, 0);
    WINDOW* info_win = newwin(LINES - split, COLS, split-1, 0);

    // Draw a box around the menu window
    box(menu_win, 0, 0);
    wborder(info_win, 0, 0, 0, 0, ACS_LTEE, ACS_RTEE, 0, 0);

    // Display the menu and refresh the screen
    wrefresh(menu_win);
    wrefresh(info_win);
    refresh();

    getch();
    endwin();
}