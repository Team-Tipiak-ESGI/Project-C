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
// permet de determiner la taille du tableau en divisant la taille max du tab par le ptr = 0 du dit tableau
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))



void window_items(SSL *ssl){
    ITEM **fichiersUpload, **fichiersDownload;
    int character, download_choices, upload_choices;
    MENU *menu_fichiers_upload, *menu_fichiers_download;
    WINDOW *win_left_panel, *win_top_panel, *win_upload_panel, *sub_win_upload_panel,
            *win_download_panel, *sub_win_download_panel;
    PANEL *panel_main[3];

    //init ecran et fenetres
    initscr();

    cbreak(); //pas de line buffering, tout sera passe au programme
    keypad(stdscr, TRUE); // touches fonctions
    noecho(); // pas de texte au clavier quand on fait des input;

    // creation des windows avant entree dans les panels
    win_left_panel = newwin(LINES, RATIOX, 0, 0); // left panel
    win_top_panel = newwin(RATIOY, COLS, 0, 0); // top panel

    // box autour des win_*_panels
    box(win_top_panel, 0, 0);
    box(win_left_panel, 0, 0);

    // mise dans des panels
    panel_main[0] = new_panel(win_left_panel);
    panel_main[1] = new_panel(win_top_panel);
    panel_main[2] = NULL;


    //<------------------ DOWNLOAD ---------------------->

    // creation items pour download
    download_choices = ARRAY_SIZE(listFiles(ssl));
    fichiersDownload = (ITEM **)calloc(download_choices, sizeof(ITEM *));
    // insertion des fichiers dans une liste
    for (int i = 0; i < download_choices; ++i) {
        fichiersDownload[i] = new_item(listFiles(ssl)[i], listFiles(ssl)[i]);
    }

    // creation du menu
    menu_fichiers_download = new_menu((ITEM **)fichiersDownload);

    //creation fenetres
    win_download_panel = newwin(20, 76, (LINES + 15) / 2, (COLS - 76) / 2); // password_panel
    sub_win_download_panel = derwin(win_download_panel, 15, 70, 3, 3);

    // ajout box et controle keypad
    box(win_download_panel, 0, 0);
    box(sub_win_download_panel, 0, 0);
    keypad(win_download_panel, TRUE);
    keypad(sub_win_download_panel, TRUE);

    //placement fenetres qui vont se placer dans le menu
    set_menu_win(menu_fichiers_download, sub_win_download_panel);
    set_menu_sub(menu_fichiers_download, derwin(sub_win_download_panel,10, 60, 2, 2));
    set_menu_format(menu_fichiers_download, 5, 1);

    //ajout markeur pour dire si le fichier a ete pointe ou non
    set_menu_mark(menu_fichiers_download," * ");

    // modification des param avant pubage du menu
    box(menu_fichiers_download, 0,0);
    post_menu(menu_fichiers_download);
    wrefresh(win_download_panel);

    while ((character == wgetch(sub_win_download_panel)) != KEY_F(4)){
        switch (character) {
            case KEY_DOWN:
                menu_driver(menu_fichiers_download, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(menu_fichiers_download, REQ_UP_ITEM);
                break;
            // faire add, del, upload, switch entre up et download, squelette fini
        }
    }
}

int window_login(SSL *ssl) {
    // capture claver -- ascii table
    int character;
    int choices = 0;

    //init ecran et fenetres
    initscr();

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
    field_signup[0] = new_field(1, 10, 0, 0, 0, 0); // label id
    field_signup[1] = new_field(1, 30, 0, 15, 0, 0); // id
    field_signup[2] = new_field(1, 10, 2, 0, 0, 0); // label pass
    field_signup[3] = new_field(1, 30, 2, 15, 0, 0); // pass
    field_signup[4] = new_field(1, 10, 4, 0, 0, 0); // label pass confirm
    field_signup[5] = new_field(1, 30, 4, 15, 0, 0); // pass confirm
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
    field_login[0] = new_field(1, 10, 0, 0, 0, 0); // label id
    field_login[1] = new_field(1, 30, 0, 15, 0, 0); // id
    field_login[2] = new_field(1, 10, 2, 0, 0, 0); // label pass
    field_login[3] = new_field(1, 30, 2, 15, 10, 0); // pass
    field_login[4] = NULL;//new_field(1,4,8,30,0,0); // <OK>
    field_login[5] = NULL;


    //mise en place des textes labels -- login
    set_field_buffer(field_login[0], 0, "login:");
    set_field_buffer(field_login[2], 0, "password:");
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


    while ((character = wgetch(((choices == 1) ? sub_win_signup_panel : sub_win_login_panel))) != KEY_F(4)) {

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
            wrefresh(((choices == 1) ? sub_win_signup_panel : sub_win_login_panel)));
        }
        if (character == KEY_F(2) && choices == 0) {
            set_current_field(form_signup, field_signup[1]);
            choices = 1;
        } else if (choices == 1 && character == KEY_F(2)) {
            set_current_field(form_login, field_login[1]);
            choices = 0;
        }
        if (choices == 1 && (character == 10 || character == KEY_ENTER)) {
            if (!strcmp(field_signup[3], field_signup[5]))
                if (signup(ssl, field_signup[1], field_signup[3]))
                    break;
        } else if (choices == 0 && (character == 10 || character == KEY_ENTER)) {
            if (login(ssl, field_login[1], field_login[3]))
                break;
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