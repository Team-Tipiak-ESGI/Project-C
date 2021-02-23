// ncurses et ses extensions
#include <curses.h>
#include <panel.h>
#include <menu.h>
#include <form.h>

#include <stdlib.h>
#include <string.h>

#include <openssl/bio.h>

//parce que les structures de generation de fenetres
//seront toutes les memes
#include "window.h"
#include "SendFile.h"
#include "Connection.h"

// definition hauteur / largeur par defaut
#define WIDTH 30
#define HEIGHT 10
// ratio pour les panels
#define RATIOY (LINES * 0.2)
#define RATIOX (COLS * 0.1)

int window(SSL*ssl){
    // generation SSL pour login / signup
    // capture claver -- ascii table
    int character;
    int choices = 0;

    //init ecran et fenetres
    initscr();

    if (has_colors() == FALSE){
        perror("ERROR : ");
    } else {
        start_color(); // couleurs
        init_pair(1, COLOR_WHITE, COLOR_BLUE); // couleurs sur le terminal, texte BLANC, fond BLEU
        init_pair(2, COLOR_WHITE, COLOR_BLUE); // couleurs sur le terminal, texte BLANC, fond BLEU
        // 2 PAIRS identique pour formulaire
        init_pair(3, COLOR_GREEN, COLOR_BLACK); // couleurs sur le terminal, texte VERT, fond NOIR
        init_pair(4, COLOR_GREEN, COLOR_BLACK); // couleurs sur le terminal, texte VERT, fond NOIR
    }

    cbreak(); //pas de line buffering, tout sera passe au programme
    keypad(stdscr, TRUE); // touches fonctions
    noecho(); // pas de texte au clavier quand on fait des input;

    // INIT du panel
    WINDOW *win_left_panel, *win_top_panel, *sub_win_login_panel, *sub_win_signup_panel;
    PANEL *panel_main[3];

    // initialialisation formulaires

    FORM *form_login;
    FORM *form_signup;

    // creation des windows avant entree dans les panels
    win_left_panel = newwin(LINES, RATIOX, 0, 0); // left panel
    win_top_panel = newwin(RATIOY, COLS, 0, 0); // top panel

    // box autour des win_*_panels
    box(win_top_panel, 0, 0);
    box(win_left_panel, 0, 0);


    // insertion dans les panels
    // ordre des panels -> stdscr-0-1-2-3
    // panel 2 et 3 - login / signup

    panel_main[0] = new_panel(win_left_panel);
    panel_main[1] = new_panel(win_top_panel);
    panel_main[2] = NULL;

// <---------- PORTION SIGNUP ----------->
    //mise a jour des panels pour finir la creation
    update_panels();
    // affichage sur ecran
    doupdate();
    // boucle manipulation fenetre au clavier

    // init du form inscription
    WINDOW *win_signup_panel;
    FIELD *field_signup[8];

    // creation des fenetres avant entree dans panels
    win_signup_panel = newwin(20, 76, (LINES + 15) / 2, (COLS - 76) / 2); // password_panel
    sub_win_signup_panel = derwin(win_signup_panel, 15, 70, 3, 3);

    mvwprintw(win_signup_panel, 1, 2, "Press F4 to quit - F2 to switch between login / sign up forms");

    // creation contours
    box(win_signup_panel, 0, 0);
    box(sub_win_signup_panel, 0, 0);
    keypad(win_signup_panel, TRUE);
    keypad(sub_win_signup_panel, TRUE);

    // initialisation des champs de signup
    field_signup[0] = new_field(1,10,0,0,0,0); // label id
    field_signup[1] = new_field(1,30,0,15,0,0); // id
    field_signup[2] = new_field(1,10,2,0,0,0); // label pass
    field_signup[3] = new_field(1,30,2,15,0,0); // pass
    field_signup[4] = new_field(1,10,4,0,0,0); // label pass confirm
    field_signup[5] = new_field(1,30,4,15,0,0); // pass confirm
    field_signup[6] = NULL;//new_field(1,4,8,30,0,0); // <OK>
    field_signup[7] = NULL;

    //mise en place des textes labels -- signup
    set_field_buffer(field_signup[0], 0, "login:");
    set_field_buffer(field_signup[2], 0, "password:");
    set_field_buffer(field_signup[4], 0, "confirm:");
    //set_field_buffer(field_signup[6], 0,"<OK>");

    // CUSTOMISATION DES CHAMPS
    // label id
    set_field_opts(field_signup[0], O_AUTOSKIP | O_PUBLIC | O_VISIBLE);
    // id
    set_field_just(field_signup[1], JUSTIFY_CENTER);
    field_opts_off(field_signup[1], O_NULLOK);
    set_field_opts(field_signup[1], O_PUBLIC | O_VISIBLE | O_ACTIVE | O_EDIT);
    set_field_back(field_signup[1], A_UNDERLINE);
    set_field_type(field_signup[1], TYPE_ALNUM);
    //label pass
    set_field_opts(field_signup[2], O_AUTOSKIP | O_PUBLIC | O_VISIBLE);
    // pass
    set_field_opts(field_signup[3], O_VISIBLE | O_ACTIVE | O_EDIT);
    field_opts_off(field_signup[3], O_PUBLIC | O_NULLOK);
    set_field_back(field_signup[3], A_UNDERLINE);
    set_field_type(field_signup[3], TYPE_ALNUM);
    //label pass confirm
    set_field_opts(field_signup[4], O_AUTOSKIP | O_PUBLIC | O_VISIBLE);
    // pass confirm
    set_field_opts(field_signup[5], O_VISIBLE | O_ACTIVE | O_EDIT);
    field_opts_off(field_signup[5], O_PUBLIC);
    set_field_back(field_signup[5], A_UNDERLINE);
    set_field_type(field_signup[5], TYPE_ALNUM);

    // creation formulaire inscription
    form_signup = new_form(field_signup);

    // integration formulaires dans des fenetres
    set_form_win(form_signup, sub_win_signup_panel);
    set_form_sub(form_signup, derwin(sub_win_signup_panel, 10, 60, 2, 2));

    //pub form signup
    post_form(form_signup);
    refresh();
    wrefresh(win_signup_panel);
    wrefresh(sub_win_signup_panel);
// <---------------------- portion LOGIN -------------------->
    WINDOW *win_login_panel;
    FIELD *field_login[6];

    win_login_panel = newwin(20, 76, (LINES - 30) / 2, (COLS - 76) / 2); // login_panel // body
    sub_win_login_panel = derwin(win_login_panel, 15, 70, 3, 3); // sub_panel // form
    mvwprintw(win_login_panel, 1, 2, "Press F4 to quit - F2 to switch between login / sign up forms");

    box(win_login_panel, 0, 0);
    box(sub_win_login_panel, 0, 0);
    keypad(win_login_panel, TRUE);
    keypad(sub_win_login_panel, TRUE);

    // initialisation des champs de login
    field_login[0] = new_field(1,10,0,0,0,0); // label id
    field_login[1] = new_field(1,30,0,15,0,0); // id
    field_login[2] = new_field(1,10,2,0,0,0); // label pass
    field_login[3] = new_field(1,30,2,15,10,0); // pass
    field_login[4] = NULL;//new_field(1,4,8,30,0,0); // <OK>
    field_login[5] = NULL;


    //mise en place des textes labels -- login
    set_field_buffer(field_login[0],0,"login:");
    set_field_buffer(field_login[2],0,"password:");
    //set_field_buffer(field_login[4],0,"<OK>");

    // CUSTOMISATION DES CHAMPS
    // label id
    set_field_opts(field_login[0], O_AUTOSKIP | O_PUBLIC | O_VISIBLE);
    // id
    set_field_just(field_login[1], JUSTIFY_CENTER);
    field_opts_off(field_login[1], O_NULLOK);
    set_field_opts(field_login[1], O_PUBLIC | O_VISIBLE | O_ACTIVE | O_EDIT);
    set_field_back(field_login[1], A_UNDERLINE);
    set_field_type(field_login[1], TYPE_ALNUM);
    //label pass
    set_field_opts(field_login[2], O_AUTOSKIP | O_PUBLIC | O_VISIBLE);
    // pass
    set_field_opts(field_login[3], O_VISIBLE | O_ACTIVE | O_EDIT);
    field_opts_off(field_login[3], O_PUBLIC | O_NULLOK);
    set_field_back(field_login[3], A_UNDERLINE);
    set_field_type(field_login[3], TYPE_ALNUM);

    // creation du formulaire de connexion / login.
    form_login = new_form(field_login);

    // integration formulaires dans fenetres
    set_form_win(form_login, sub_win_login_panel);
    set_form_sub(form_login, derwin(sub_win_login_panel, 10, 60, 2, 2));

    // publication formulaire login + signup
    post_form(form_login);

    set_current_field(form_login, field_login[1]);
    refresh();
    wrefresh(win_login_panel);
    wrefresh(sub_win_login_panel);


    while((character = wgetch(((choices == 1 ) ? sub_win_signup_panel : sub_win_login_panel))) != KEY_F(4)) {

        if (!choices) {
            switch (character) {
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
                default:
                    form_driver(form_login, character);
                    break;
                }
            } else {
                switch (character) {
                    case KEY_LEFT:
                        form_driver(form_signup, REQ_PREV_CHAR);
                        break;
                    case KEY_RIGHT:
                        form_driver(form_signup, REQ_NEXT_CHAR);
                        break;
                        // deplacement dans le formulaire
                    case KEY_UP:
                        form_driver(form_signup, REQ_PREV_FIELD);
                        form_driver(form_signup, REQ_END_LINE);
                        break;
                    case KEY_DOWN:
                        form_driver(form_signup, REQ_NEXT_FIELD);
                        form_driver(form_signup, REQ_END_LINE);
                        break;
                        // Supprime le caractere qui est avant le curseur
                    case KEY_BACKSPACE:
                    case 126:
                        form_driver(form_signup, REQ_DEL_PREV);
                        break;
                        // Supprime le caractere qui est sur le curseur
                    case KEY_DC:
                        form_driver(form_signup, REQ_DEL_CHAR);
                        break;
                        // Remonter au debut ou a la fin
                    case KEY_HOME:
                        form_driver(form_signup, REQ_BEG_LINE);
                        break;
                    case KEY_END:
                        form_driver(form_signup, REQ_END_LINE);
                        break;
                    default:
                        form_driver(form_signup, character);
                        break;
            }
        }
        if (character == KEY_F(2) && choices == 0) {
            set_current_field(form_signup, field_signup[1]);
            choices = 1;
        } else if (choices == 1 && character == KEY_F(2)){
            set_current_field(form_login, field_login[1]);
            choices = 0;
        }
        if (character == KEY_ENTER && choices == 1) {
            if(strcmp(field_signup[3],field_signup[5]))
                signup(ssl, field_signup[1], field_signup[3]);
        } else if (choices == 0 && character == KEY_ENTER) {
            login(ssl, field_login[1], field_login[3]);
        }
    }


    // depubliage du formulaire
    unpost_form(form_login);
    //liberation memoire
    free_form(form_login);
    free_form(form_signup);
    endwin();
    return EXIT_SUCCESS;
}