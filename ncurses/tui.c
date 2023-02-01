#include <menu.h>
#include <form.h>
#include <panel.h>
#include <string.h> // strlen
#include <stdlib.h>
#include <periodic.h>

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

void addElementMenu(PANEL* form_panel, WINDOW* form_win, FORM* add_form) {
    int ch, add_lines, add_cols;
    FIELD** add_fields = form_fields(add_form);
    scale_form(add_form, &add_lines, &add_cols); // Get the size of the form
    show_panel(form_panel);
    update_panels();
    doupdate();
    while ((ch = wgetch(form_win)) != KEY_F(1)) {
        switch (ch) {
            case 9:
            case KEY_DOWN:
                form_driver(add_form, REQ_NEXT_FIELD);
                form_driver(add_form, REQ_END_LINE);
                break;
            case KEY_UP:
                form_driver(add_form, REQ_PREV_FIELD);
                form_driver(add_form, REQ_END_LINE);
                break;
			case 127: // Also backspace on different systems, possibly also KEY_DC
            case KEY_BACKSPACE:
                form_driver(add_form, REQ_PREV_CHAR);
                form_driver(add_form, REQ_DEL_CHAR);
                break;
            case 10: // Enter
                if ((form_driver(add_form, REQ_VALIDATION) != E_OK) || !field_status(add_fields[0]) || !field_status(add_fields[2])) {
                    midPrint(form_win, 1, 0, add_cols + 4, "Wrong Data!", COLOR_PAIR(2));
                    // sleep(5);
                    pos_form_cursor(add_form);
                    update_panels();
                    doupdate();
                } else {
                    goto exit_loop;
                }
                break;
            default:
                // Print a normal character
                form_driver(add_form, ch);
                break;
        }
    }
    // Macro for leaving the loop from inside switch case
    exit_loop: ;
}

void searchElementMenu(PANEL* s_panel, WINDOW* s_win, MENU* s_menu) {
    int ch;
    char* str = malloc(sizeof(char) * 20);
    post_menu(s_menu);
    show_panel(s_panel);
    update_panels();
    doupdate();
    while ((ch = wgetch(s_win)) != KEY_F(2)) {
        switch (ch) {
            case KEY_RIGHT:
                menu_driver(s_menu, REQ_RIGHT_ITEM);
                break;
            case KEY_LEFT:
                menu_driver(s_menu, REQ_LEFT_ITEM);
                break;
            case 10:
                unpost_menu(s_menu);
                mvwprintw(s_win, 2, 2, "%s", "Enter query: ");
                doupdate();
                wgetnstr(s_win, str, 20);
        }
    }
}

int main(void) {
    initscr();
    refresh();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);

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

    /* CREATE ADD FORM */

    int add_cols, add_lines;
    
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

    /* Add validation */
    set_field_type(add_fields[2], TYPE_INTEGER, 3, 1, 133); // Atomic number
    set_field_type(add_fields[3], TYPE_INTEGER, 6, 1, 999999); // Atomic mass

    FORM* add_form = new_form(add_fields);
    scale_form(add_form, &add_lines, &add_cols); // get the size of the form

    WINDOW* add_form_win = newwin(add_lines + 4, add_cols + 4, (LINES / 2) - 10 - (add_lines / 2), (COLS / 2) - (add_cols / 2));
    keypad(add_form_win, TRUE);
    PANEL* add_form_panel = new_panel(add_form_win);

    set_form_win(add_form, add_form_win);
    set_form_sub(add_form, derwin(add_form_win, add_lines, add_cols, 2, 2));

    /* Styling */
    box(add_form_win, 0, 0);
    midPrint(add_form_win, 1, 0, add_cols + 4, "Add Element", COLOR_PAIR(1));

    post_form(add_form);
    hide_panel(add_form_panel);

    /* CREATE SEARCH */
    int s_lines, s_cols;
    ITEM* s_items[5];
    char* choices[] = {"Name", "Symbol", "Atomic Num", "Atomic Mass", (char*)NULL};

    for (int i = 0; i < 5; i++) {
        s_items[i] = new_item(choices[i], choices[i]);
        set_item_userptr(s_items[i], i);
    }
    MENU* s_menu = new_menu(s_items);
    menu_opts_off(s_menu, O_SHOWDESC);
    set_menu_mark(s_menu, " * ");
    scale_menu(s_menu, &s_lines, &s_cols);

    WINDOW* s_menu_win = newwin(s_lines + 4, s_cols + 4, (LINES / 2) - 10 - (s_lines / 2), (COLS / 2) - (s_cols / 2));
    keypad(s_menu_win, TRUE);
    PANEL* s_menu_panel = new_panel(s_menu_win);

    set_menu_win(s_menu, s_menu_win);
    set_menu_sub(s_menu, derwin(s_menu_win, s_lines, s_cols, 2 ,2));

    box(s_menu_win, 0, 0);
    midPrint(s_menu_win, 1, 0, s_cols + 4, "Search for Element", COLOR_PAIR(1));

    hide_panel(s_menu_panel);

    update_panels();
    doupdate();

    int ch;
    while((ch = getch()) != 113) {
        switch (ch) {
            case KEY_F(1):
                addElementMenu(add_form_panel, add_form_win, add_form);
                hide_panel(add_form_panel);
                update_panels();
                doupdate();
                break;
            case KEY_F(3):
                
            case KEY_F(10):
                endwin();
                exit(0);
            default:
                break;
        }
    }

    getch();

    show_panel(add_form_panel);
    update_panels();
    doupdate();
    getch();
    endwin();
}