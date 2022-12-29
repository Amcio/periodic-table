#include <menu.h>
#include <form.h>
#include <panel.h>
#include <string.h> // strlen

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
    length = strlen(string);
    temp = (width - length) / 2; // Half the space left
    x = startx + (int)temp; // Move our string to the center
    wattron(win, color);
    mvwprintw(win, y, x, "%s", string);
    wattroff(win, color);
    refresh();
}

int main(void) {
    initscr();
    refresh();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    init_pair(1, COLOR_CYAN, COLOR_BLACK);

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

    getch();

    int add_cols, add_lines, ch;
    
    FIELD* add_fields[6];
    add_fields[0] = new_field(1, 20, 1, 1, 0, 0); // Element name
    add_fields[1] = new_field(1, 4, 3, 1, 0, 0); // Element symbol
    add_fields[2] = new_field(1, 4, 5, 1, 0, 0); // Atomic number
    add_fields[3] = new_field(1, 6, 7, 1, 0 ,0); // Atomic mass
    add_fields[4] = new_field(1, 40, 9, 1, 0, 0); // Comment
    add_fields[5] = NULL; // NULL-terminated
    for (int i = 0; i < 5; i++) {
        set_field_back(add_fields[i], A_UNDERLINE);
        field_opts_off(add_fields[i], O_AUTOSKIP);
    }

    FORM* add_form = new_form(add_fields);
    scale_form(add_form, &add_lines, &add_cols);

    WINDOW* add_form_win = newwin(add_lines + 4, add_cols + 4, 4, 6);
    keypad(add_form_win, TRUE);
    PANEL* add_form_panel = new_panel(add_form_win);

    set_form_win(add_form, add_form_win);
    set_form_sub(add_form, derwin(add_form_win, add_lines, add_cols, 2, 2));

    /* Styling */
    box(add_form_win, 0, 0);
    midPrint(add_form_win, 1, 0, add_cols + 4, "Add Element", COLOR_PAIR(1));

    post_form(add_form);
    update_panels();

    doupdate();

    while ((ch = wgetch(add_form_win)) != KEY_F(1)) {
        switch (ch)
        {
        case KEY_DOWN:
            form_driver(add_form, REQ_NEXT_FIELD);
            form_driver(add_form, REQ_END_LINE);
            break;
        case KEY_UP:
            form_driver(add_form, REQ_PREV_FIELD);
            form_driver(add_form, REQ_END_LINE);
            break;
        case KEY_BACKSPACE:
            form_driver(add_form, REQ_PREV_CHAR);
            form_driver(add_form, REQ_DEL_CHAR);
            break;
        default:
            // Print a normal character
            form_driver(add_form, ch);
            break;
        }
    }

    endwin();
}