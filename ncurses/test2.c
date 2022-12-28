#include <ncurses.h>
#include <menu.h>
#include <stdlib.h>
#include <string.h>

#define ARRSIZE(a) (sizeof(a) / sizeof(a[0]))
char* choices[] = {
    "Add an Element",
    "Remove an Element",
    "Print all Elements",
    "Exit",
};

void midPrint(WINDOW* win, int starty, int startx, int width, char* string, chtype color) {
    int length, y, x;
    float temp;

    if (win == NULL) {
        win = stdscr;
    }
    getyx(win, y, x);
    if (startx != 0) {
        x = startx;
    }
    if (starty != 0) {
        y = starty;
    }
    if (width == 0) {
        width = 80;
    }
    length = strlen(string);
    temp = (width - length) / 2; // Half the space left
    x = startx + (int)temp; // Move our string to the center
    wattron(win, color);
    mvwprintw(win, y, x, "%s", string);
    wattroff(win, color);
    refresh();
}

int main(void) {
    ITEM **my_items; // Pointer which points to pointers which hold items.
    int c; // Character for getch()
    MENU* my_menu;
    ITEM* cur_item;

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    start_color();
    init_pair(1, COLOR_BLUE, COLOR_BLACK);

    int n_choices = ARRSIZE(choices);
    my_items = (ITEM**)calloc(n_choices + 1, sizeof(ITEM*)); // Allocate for our choices + a NULL var
    for (int i = 0; i < n_choices; i++) {
        my_items[i] = new_item(choices[i], "");
    }
    my_items[n_choices] = (ITEM*)NULL;
    my_menu = new_menu((ITEM**)my_items);

    set_menu_mark(my_menu, "*");
    WINDOW* menu_win = newwin(10, 40, 4, 4);
    keypad(menu_win, TRUE);
    /* Set main window and sub window */
    set_menu_win(my_menu, menu_win);
    set_menu_sub(my_menu, derwin(menu_win, 6, 38, 3, 1));
    box(menu_win, 0, 0);
    midPrint(menu_win, 1, 0, 40, "Periodic Table", COLOR_PAIR(1));
    mvwaddch(menu_win, 2, 0, ACS_LTEE);
    mvwhline(menu_win, 2, 1, ACS_HLINE, 38);
    mvwaddch(menu_win, 2, 39, ACS_RTEE);
    refresh();
    post_menu(my_menu);
    wrefresh(menu_win);
    while ((c = wgetch(menu_win)) != 113) {
        switch (c) {
            case KEY_DOWN:
                menu_driver(my_menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(my_menu, REQ_UP_ITEM);
                break;
        }
        wrefresh(menu_win);
    }
    unpost_menu(my_menu);
    free_menu(my_menu);
    for(int i = 0; i < n_choices; i++) {
        free_item(my_items[i]);
    }
    endwin();
}
