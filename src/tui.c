#include <menu.h>
#include <form.h>
#include <panel.h>
#include <string.h> // strlen
#include <stdlib.h>
#include "periodic.h"

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

void recreateElementMenu(MENU* elements_menu, element** Elements, size_t* n_elements) {
    unpost_menu(elements_menu);
    ITEM** elements_items = menu_items(elements_menu);
    size_t old_n_elements = *n_elements;

    freeElements(*Elements, *n_elements);
    *Elements = readElements(n_elements);
    ITEM** new_elements_items = (ITEM**)calloc((*n_elements) + 1, sizeof(ITEM*));
    // Generation of this array could possibly be in another function. We would call it here, and for creation of the menu
    for (int i = 0; i < *n_elements; i++) {
        new_elements_items[i] = new_item((*Elements)[i].symbol, (*Elements)[i].name);
        set_item_userptr(new_elements_items[i], &(*Elements)[i]);
    }
    new_elements_items[(*n_elements)] = (ITEM*)NULL; // null-terminated
    set_menu_items(elements_menu, new_elements_items);
    /*
    The old Elements array is still used in some way while calling set_menu_items().
    We can only free it AFTER we swapped them.
    */
    for (int i = 0; elements_items[i]; i++) {
        free_item(elements_items[i]);
    }
    memset(elements_items, '\0', old_n_elements);
    free(elements_items);
    post_menu(elements_menu);
}

int addElementMenu(PANEL* form_panel, WINDOW* form_win, FORM* add_form) {
    int ch, add_lines, add_cols, added = 0;
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
			case 8: // Windows
            case KEY_BACKSPACE:
                form_driver(add_form, REQ_PREV_CHAR);
                form_driver(add_form, REQ_DEL_CHAR);
                break;
            case 10: // Enter
                if ((form_driver(add_form, REQ_VALIDATION) != E_OK) || !strcmp("", strstrip(field_buffer(add_fields[1], 0))) || !strcmp("", strstrip(field_buffer(add_fields[0], 0))) || !field_status(add_fields[2]) || !field_status(add_fields[3])) {
                    midPrint(form_win, 1, 0, add_cols + 4 + 8, "Wrong Data!", COLOR_PAIR(2));
                    // sleep(5);
                    pos_form_cursor(add_form);
                    wrefresh(form_win);
                } else {
                    element Element = {.name = strstrip(field_buffer(add_fields[0], 0)),
                                    .symbol = strstrip(field_buffer(add_fields[1], 0)),
                                    .anum = strtol(strstrip(field_buffer(add_fields[2], 0)), NULL, 10),
                                    .amass = strtol(strstrip(field_buffer(add_fields[3], 0)), NULL, 10),
                                    .comment = strstrip(field_buffer(add_fields[4], 0))};
                    saveElement(&Element);
                    added = 1;
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
    for (int i = 0; i < 5; i++) {
        set_field_buffer(add_fields[i], 0, "");
    }
    form_driver(add_form, REQ_FIRST_FIELD);
    hide_panel(form_panel);
    update_panels();
    doupdate();
    if (added) {
        return 1;
    }
    return 0;

}

int32_t searchElementMenu(PANEL* s_panel, WINDOW* s_win, MENU* s_menu, element* Elements, size_t length) {
    int ch, offset, s_cols, s_lines;
    char str[20] = "NULL";
    scale_menu(s_menu, &s_lines, &s_cols);

    werase(s_win);
    box(s_win, 0, 0);
    midPrint(s_win, 1, 0, s_cols + 11, "Search", COLOR_PAIR(1));
    post_menu(s_menu);
    show_panel(s_panel);
    update_panels();
    wnoutrefresh(s_win);
    doupdate();
    while ((ch = wgetch(s_win)) != KEY_F(3)) {
        switch (ch) {
            case KEY_UP:
                menu_driver(s_menu, REQ_UP_ITEM);
                break;
            case KEY_DOWN:
                menu_driver(s_menu, REQ_DOWN_ITEM);
                break;
            case 10: /* Enter */
                offset = item_index(current_item(s_menu));
                unpost_menu(s_menu);
                mvwprintw(s_win, 2, 5, "%s", "Enter query: \n");
                box(s_win, 0, 0); // redraw box after newline
                doupdate();
                echo();
                wmove(s_win, 3, 1);
                wgetnstr(s_win, str, 20);
                noecho();
                goto exit_loop;
                break;
        }
    }
    exit_loop: ;
    unpost_menu(s_menu); // The menu was not necessarily unposted on line 98. Make sure it is.
    hide_panel(s_panel);
    update_panels();
    doupdate();
    if (offset == 2 || offset == 3) {
        int query = strtol(str, NULL, 10);
        return searchElement(Elements, length, &query, offset);
    }

    return searchElement(Elements, length, str, offset);
}

int removeElementMenu(PANEL* remove_panel, WINDOW* remove_win, FORM* remove_form, element* Elements, size_t n_elements) {
    int ch, removed = 0;
    FIELD** remove_fields = form_fields(remove_form);
    show_panel(remove_panel);
    update_panels();
    doupdate();
    while ((ch =  wgetch(remove_win)) != KEY_F(2)) {
        switch (ch) {
            case 127:
			case 8: // Windows
            case KEY_BACKSPACE:
                form_driver(remove_form, REQ_PREV_CHAR);
                form_driver(remove_form, REQ_DEL_CHAR);
                break;
            case 10:
                form_driver(remove_form, REQ_VALIDATION);
                if (!field_status(remove_fields[0])) {
                    continue;
                }
                int elem = searchElement(Elements, n_elements, strstrip(field_buffer(remove_fields[0], 0)), 0);
                if (elem < 0) {
                    form_driver(remove_form, REQ_CLR_FIELD);
                    continue;
                }
                removeElement(&Elements[elem]);
                removed = 1;
                goto exit_loop;
                break;
            default:
                form_driver(remove_form, ch); // Print normal character
                break;
        }
    }
    exit_loop: ;
    form_driver(remove_form, REQ_CLR_FIELD);
    hide_panel(remove_panel);
    update_panels();
    doupdate();
    if (removed) {
        return 1;
    }
    return 0;
}

FORM* createFilterForm() {
    int f_lines, f_cols;
    FIELD** filter_fields = malloc(sizeof(FIELD*) * 2); // Has to be malloc'ed, otherwise the variable is freed when function exits.
    filter_fields[0] = new_field(1, 4, 1, 8, 0, 0);
    filter_fields[1] = NULL;

    set_field_back(filter_fields[0], A_UNDERLINE);
    field_opts_off(filter_fields[0], O_AUTOSKIP);
    set_field_type(filter_fields[0], TYPE_INTEGER, 3, 0, 133); // Validation for atomic number, same as in add_form

    FORM* filter_form = new_form(filter_fields);
    scale_form(filter_form, &f_lines, &f_cols);

    WINDOW* filter_form_win = newwin(f_lines + 5, 25, LINES / 2 - 10 - (f_lines / 2), (COLS / 2) - (25 / 2));
    keypad(filter_form_win, TRUE);
    // Panel has to be created in main, because we can not get the panel from the window. We can only get the window from the panel.
    // Creating a panel here would require us to return two variables.
    set_form_win(filter_form, filter_form_win);
    set_form_sub(filter_form, derwin(filter_form_win, f_lines, f_cols, 3, 2));

    box(filter_form_win, 0, 0);
    midPrint(filter_form_win, 1, 0, 25, "Filter", COLOR_PAIR(1));
    midPrint(filter_form_win, 2, 0, 25, "Enter 0 to reset", COLOR_PAIR(99));
    wnoutrefresh(filter_form_win);
    post_form(filter_form);

    return filter_form;
}

int filterElementsMenu(PANEL* filter_panel, FORM* filter_form) {
    int ch, filter = -1;
    WINDOW* filter_win = form_win(filter_form);
    FIELD** filter_fields = form_fields(filter_form);
    show_panel(filter_panel);
    update_panels();
    doupdate();
    while ((ch = wgetch(filter_win)) != KEY_F(4)) {
        switch (ch) {
            case 127:
			case 8: // Windows
            case KEY_BACKSPACE:
                form_driver(filter_form, REQ_PREV_CHAR);
                form_driver(filter_form, REQ_DEL_CHAR);
                break;
            case 10: // Enter
                if (form_driver(filter_form, REQ_VALIDATION) != E_OK) {
                    form_driver(filter_form, REQ_CLR_FIELD);
                    continue;
                }
                char* buf = field_buffer(filter_fields[0], 0);
                filter = strtol(strstrip(buf), NULL, 10);
                goto exit_loop;
                break;
            default:
                form_driver(filter_form, ch);
                break;
        }
    }
    exit_loop: ;
    form_driver(filter_form, REQ_CLR_FIELD);
    hide_panel(filter_panel);
    update_panels();
    doupdate();

    return filter;
}

void filterElements(MENU* elements_menu, element** Elements, size_t* n_elements, int filter) {
    unpost_menu(elements_menu);
    ITEM** elements_items = menu_items(elements_menu);
    size_t old_n_elements = *n_elements, new_n_elements = 0;
    element* newElements = NULL;

    for (int i = 0; i < old_n_elements; i++) {
        if ((*Elements)[i].anum <= filter) {
            newElements = (element*)realloc(newElements, sizeof(element) * (new_n_elements + 1));
            newElements[new_n_elements] = (*Elements)[i];
            new_n_elements++;
        } else {
            freeElement(&(*Elements)[i]);
        }
    }
    *n_elements = new_n_elements;
    free(*Elements); // This could potentially free ALL elements.
    *Elements = newElements;
    ITEM** new_elements_items = (ITEM**)calloc((new_n_elements) + 1, sizeof(ITEM*));
    // Generation of this array could possibly be in another function. It is now done thrice.
    for (int i = 0; i < new_n_elements; i++) {
        new_elements_items[i] = new_item((*Elements)[i].symbol, (*Elements)[i].name);
        set_item_userptr(new_elements_items[i], &(*Elements)[i]);
    }
    new_elements_items[(new_n_elements)] = (ITEM*)NULL; // null-terminated
    set_menu_items(elements_menu, new_elements_items);
    /*
    The old Elements array is still used in some way while calling set_menu_items().
    We can only free it AFTER we swapped them.
    */
    for (int i = 0; elements_items[i]; i++) {
        free_item(elements_items[i]);
    }
    memset(elements_items, '\0', old_n_elements);
    free(elements_items);
    post_menu(elements_menu);
}

void printElementInfo(WINDOW* info_win, MENU* element_menu, element* Elements) {
    werase(info_win);
    ITEM* cur_item = current_item(element_menu);
    element Element = *(element*)item_userptr(cur_item);
    mvwprintw(info_win, 0, 0, "Name: %s\nSymbol: %s\nAtomic Number: %d\nAtomic Mass: %d\nComment: %s", Element.name, Element.symbol, Element.anum, Element.amass, Element.comment);
    wrefresh(info_win);
    pos_menu_cursor(element_menu);
}

int main(void) {
    initscr();
    refresh(); // Things will break otherwise
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(99, COLOR_WHITE, COLOR_BLACK);

    int split = (0.75 * LINES) - 1; // Point of window split in 2/3 of the screen
    WINDOW* menu_win = newwin(split, COLS, 0, 0);
    WINDOW* info_win = newwin(LINES - split, COLS, split-1, 0);
    WINDOW* info_txt = derwin(info_win, LINES - split - 3, COLS - 3, 1, 1);
    keypad(menu_win, TRUE);

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
    set_field_type(add_fields[0], TYPE_ALPHA, 1); // Element name
    set_field_type(add_fields[1], TYPE_ALPHA, 1); // Element symbol !Importante
    set_field_type(add_fields[2], TYPE_INTEGER, 3, 1, 133); // Atomic number
    set_field_type(add_fields[3], TYPE_INTEGER, 6, 1, 999999); // Atomic mass

    FORM* add_form = new_form(add_fields);
    scale_form(add_form, &add_lines, &add_cols); // get the size of the form

    WINDOW* add_form_win = newwin(add_lines + 4, add_cols + 4 + 8, (LINES / 2) - 10 - (add_lines / 2), (COLS / 2) - (add_cols / 2) - 8);
    keypad(add_form_win, TRUE);
    PANEL* add_form_panel = new_panel(add_form_win);

    set_form_win(add_form, add_form_win);
    set_form_sub(add_form, derwin(add_form_win, add_lines, add_cols, 2, 9));
    mvwaddstr(add_form_win, 3, 2, "Name");
    mvwaddstr(add_form_win, 5, 2, "Symbol");
    mvwaddstr(add_form_win, 7, 2, "A. Num");
    mvwaddstr(add_form_win, 9, 2, "A. Mass");
    mvwaddstr(add_form_win, 11, 2, "Comment");

    /* Styling */
    box(add_form_win, 0, 0);
    midPrint(add_form_win, 1, 0, add_cols + 4 + 8, "Add Element", COLOR_PAIR(1));

    post_form(add_form);
    hide_panel(add_form_panel);

    /* CREATE SEARCH */
    int s_lines, s_cols;
    ITEM* s_items[5];
    char* choices[] = {"Name", "Symbol", "Atomic Num", "Atomic Mass", (char*)NULL};

    for (int i = 0; i < 5; i++) {
        s_items[i] = new_item(choices[i], choices[i]);
    }
    MENU* s_menu = new_menu(s_items);
    menu_opts_off(s_menu, O_SHOWDESC);
    set_menu_mark(s_menu, "*");
    scale_menu(s_menu, &s_lines, &s_cols);

    WINDOW* s_menu_win = newwin(s_lines + 4, s_cols + 11, (LINES / 2) - 10 - (s_lines / 2), (COLS / 2) - (s_cols / 2) - 2);
    keypad(s_menu_win, TRUE);
    PANEL* s_menu_panel = new_panel(s_menu_win);

    set_menu_win(s_menu, s_menu_win);
    set_menu_sub(s_menu, derwin(s_menu_win, s_lines, s_cols, 2 ,2));

    hide_panel(s_menu_panel);

    /* REMOVE MENU */
    int f_cols,f_lines;
    FIELD* remove_fields[2];
    remove_fields[0] = new_field(1, 20, 1, 11, 0, 0);
    remove_fields[1] = NULL;
    set_field_back(remove_fields[0], A_UNDERLINE);
    field_opts_off(remove_fields[0], O_AUTOSKIP);

    FORM* remove_form = new_form(remove_fields);
    scale_form(add_form, &f_lines, &f_cols);

    WINDOW* remove_form_win = newwin(f_lines + 4, f_cols + 4, (LINES / 2) - 10 - (f_lines / 2), (COLS / 2) - (add_cols / 2));
    keypad(remove_form_win, TRUE);
    PANEL* remove_form_panel = new_panel(remove_form_win);

    set_form_win(remove_form, remove_form_win);
    set_form_sub(remove_form, derwin(remove_form_win, f_lines, f_cols, 3, 2));

    box(remove_form_win, 0, 0);
    midPrint(remove_form_win, 1, 0, f_cols + 4, "Remove Element", COLOR_PAIR(1));
    midPrint(remove_form_win, 2, 0, f_cols + 4, "Enter name:", COLOR_PAIR(99));
    post_form(remove_form);
    hide_panel(remove_form_panel);

    /* FILTER FORM */
    FORM* filter_form = createFilterForm();
    PANEL* filter_form_panel = new_panel(form_win(filter_form));
    hide_panel(filter_form_panel);
    update_panels(); // Important call, update all panels.

    /* DISPLAY ELEMENTS */
    size_t n_elements = 0; // how many elements?
    element* Elements = readElements(&n_elements);

    ITEM** elements_items = (ITEM**)calloc(n_elements + 1, sizeof(ITEM*));
    for (int i = 0; i < n_elements; i++) {
        elements_items[i] = new_item(Elements[i].symbol, Elements[i].name);
        set_item_userptr(elements_items[i], &Elements[i]);
    }
    elements_items[n_elements] = (ITEM*)NULL; // null-terminated
    MENU* elements_menu = new_menu(elements_items);

    menu_opts_off(elements_menu, O_SHOWDESC);
    set_menu_format(elements_menu, split - 1, COLS - 4);
    set_menu_win(elements_menu, menu_win);
    set_menu_sub(elements_menu, derwin(menu_win, split - 2, COLS - 4, 1, 2));
    set_menu_mark(elements_menu, "*");
    post_menu(elements_menu);

    // Draw a box around the menu window
    wborder(menu_win, 0, 0, 0, 0, 0, 0, ACS_LTEE, ACS_RTEE);
    wborder(info_win, 0, 0, 0, 0, ACS_LTEE, ACS_RTEE, 0, 0);

    attron(A_REVERSE);
    mvprintw(LINES - 1, 0, "F1 Add F2 Remove F3 Search F4 Filter F10 Exit");
    attroff(A_REVERSE);

    wnoutrefresh(stdscr);
    wnoutrefresh(info_win);
    wnoutrefresh(menu_win);
    doupdate();

    printElementInfo(info_txt, elements_menu, Elements);

    int ch;
    while((ch = wgetch(menu_win)) != 113) {
        switch (ch) {
            case KEY_RIGHT:
                menu_driver(elements_menu, REQ_RIGHT_ITEM);
                printElementInfo(info_txt, elements_menu, Elements);
                break;
            case KEY_LEFT:
                menu_driver(elements_menu, REQ_LEFT_ITEM);
                printElementInfo(info_txt, elements_menu, Elements);
                break;
            case KEY_UP:
                menu_driver(elements_menu, REQ_UP_ITEM);
                printElementInfo(info_txt, elements_menu, Elements);
                break;
            case KEY_DOWN:
                menu_driver(elements_menu, REQ_DOWN_ITEM);
                printElementInfo(info_txt, elements_menu, Elements);
                break;
            case KEY_F(1):
                if (addElementMenu(add_form_panel, add_form_win, add_form)) {
                    recreateElementMenu(elements_menu, &Elements, &n_elements);
                    wrefresh(menu_win);
                }
                wborder(menu_win, 0, 0, 0, 0, 0, 0, ACS_LTEE, ACS_RTEE);
                wrefresh(menu_win);
                break;
            case KEY_F(2):
                if (removeElementMenu(remove_form_panel, remove_form_win, remove_form, Elements, n_elements)) {
                    recreateElementMenu(elements_menu, &Elements, &n_elements);
                    wrefresh(menu_win);
                    printElementInfo(info_txt, elements_menu, Elements);
                }
                wborder(menu_win, 0, 0, 0, 0, 0, 0, ACS_LTEE, ACS_RTEE);
                wrefresh(menu_win);
                break;
            case KEY_F(3):
                int32_t index = searchElementMenu(s_menu_panel, s_menu_win, s_menu, Elements, n_elements);
                if (index < 0) {
                    attron(COLOR_PAIR(2));
                    mvaddstr((LINES / 2) - 8, (COLS / 2) - 4, "NOT FOUND");
                    mvaddstr((LINES / 2) - 7, (COLS / 2) - 11, "PRESS ENTER TO CONTINUE");
                    attroff(COLOR_PAIR(2));
                    getch();
                    mvaddstr((LINES / 2) - 8, (COLS / 2) - 4, "         ");
                    mvaddstr((LINES / 2) - 7, (COLS / 2) - 11, "                       ");
                    wnoutrefresh(stdscr);
                    /* Potentially if we have enough elements, our error message might overwrite them, redraw the menu just in case */
                    unpost_menu(elements_menu);
                    post_menu(elements_menu);
                    wrefresh(menu_win);
                    continue;
                }
                set_current_item(elements_menu, menu_items(elements_menu)[index]);
                pos_menu_cursor(elements_menu);
                printElementInfo(info_txt, elements_menu, Elements);
                wborder(menu_win, 0, 0, 0, 0, 0, 0, ACS_LTEE, ACS_RTEE);
                wrefresh(menu_win);
                break;
            case KEY_F(4):
                int filter = 0;
                if ((filter = filterElementsMenu(filter_form_panel, filter_form)) < 0) {
                    continue; // If no filter set, nothing happens.
                } else if (filter == 0) {
                    // Remove filter
                    recreateElementMenu(elements_menu, &Elements, &n_elements);
                } else {
                    // Show only elements where atomic number < filter
                    filterElements(elements_menu, &Elements, &n_elements, filter);
                    printElementInfo(info_txt, elements_menu, Elements);
                }
		pos_menu_cursor(elements_menu);
                wborder(menu_win, 0, 0, 0, 0, 0, 0, ACS_LTEE, ACS_RTEE);
                wrefresh(menu_win);
                break;
            case KEY_F(9):
                recreateElementMenu(elements_menu, &Elements, &n_elements);
                wrefresh(menu_win);
                break;
            case KEY_F(10):
                endwin();
                exit(0);
            default:
                break;
        }
    }
    endwin();
}
