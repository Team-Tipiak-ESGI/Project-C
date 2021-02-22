// ncurses et ses extensions
#include <ncurses.h>
#include <panel.h>
#include <menu.h>
#include <form.h>

#include <stdlib.h>
#include <string.h>

static void driver_form(int character, FORM *form_login, PANEL **panels){

    switch(character) {
        case KEY_LEFT:
            form_driver(form_login, REQ_PREV_CHAR);
            break;
        case KEY_RIGHT:
            form_driver(form_login, REQ_NEXT_CHAR);
            break;
            // deplacement dans le formulaire
        case KEY_UP:
            form_driver(form_login, REQ_PREV_FIELD);
            form_driver(form_login, REQ_END_LINE);
            break;
        case KEY_DOWN:
            form_driver(form_login, REQ_NEXT_FIELD);
            form_driver(form_login, REQ_END_LINE);
            break;
            // Supprime le caractere qui est avant le curseur
        case KEY_BACKSPACE:
        case 126:
            form_driver(form_login, REQ_DEL_PREV);
            break;
            // Supprime le caractere qui est sur le curseur
        case KEY_DC:
            form_driver(form_login, REQ_DEL_CHAR);
            break;
            // Remonter au debut ou a la fin
        case KEY_HOME:
            form_driver(form_login, REQ_BEG_LINE);
            break;
        case KEY_END:
            form_driver(form_login, REQ_END_LINE);
            break;
        case KEY_F(2):
            hide_panel(panels[2]);
            show_panel(panels[3]);
            break;
        case KEY_F(3):
            hide_panel(panels[2]);
            show_panel(panels[3]);
            break;
        default:
            form_driver(form_login, character);
            break;
    }
}
