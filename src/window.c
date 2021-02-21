// ncurses et ses extensions
#include <ncurses.h>
#include <panel.h>
#include <menu.h>
#include <form.h>

#include <stdlib.h>
#include <string.h>

//parce que les structures de generation de fenetres
//seront toutes les memes
#include "window.h"
#include "driver.h"

// definition hauteur / largeur par defaut
#define WIDTH 30
#define HEIGHT 10
// ratio pour les panels
#define RATIOY (LINES * 0.2)
#define RATIOX (COLS * 0.1)

static void driver_form(int character, FORM *form_login, PANEL **panel);

int main(void){
    // capture claver -- ascii table
    int character;

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
    WINDOW *win_left_panel, *win_top_panel;
    WINDOW *win_login_panel, *sub_win_login_panel;
    WINDOW *win_signup_panel, *sub_win_signup_panel;
    PANEL *panels[7];

    // initialialisation du formulaire de connexion
    FIELD *field_login[5];
    FORM *form_login;

    // init du form inscription

     FIELD *field_signup[7];
     FORM *form_signup;


    // creation des windows avant entree dans les panels
    win_left_panel = newwin(LINES, RATIOX, 0, 0); // left panel
    win_top_panel = newwin(RATIOY, COLS, 0, 0); // top panel
    win_login_panel = newwin(20, 76, (LINES - 20)/ 2, (COLS - 76)/2); // login_panel // body
    sub_win_login_panel = derwin(win_login_panel, 15, 70, 3, 3); // sub_panel // form
    win_signup_panel = newwin(20, 76, (LINES - 40)/ 2, (COLS - 76)/2); // password_panel
    sub_win_signup_panel = derwin(win_signup_panel, 15, 70, 3, 3);

    mvwprintw(win_login_panel, 1, 2, "Press F4 to quit - F2 to switch between login / sign up forms");

    // box autour des win_*_panels
    box(win_top_panel, 0, 0);
    box(win_left_panel, 0, 0);
    box(win_login_panel, 0, 0);
    box(sub_win_login_panel, 0, 0);
    box(win_signup_panel, 0, 0);
    box(sub_win_signup_panel, 0, 0);

    // initialisation des champs de login
    field_login[0] = new_field(1,10,0,0,0,0); // label id
    field_login[1] = new_field(1,10,0,10,10,0); // id
    field_login[2] = new_field(1,10,2,0,0,0); // label pass
    field_login[3] = new_field(1,10,2,10,10,0); // pass
    field_login[4] = NULL;

    // initialisation des champs de login
    field_signup[0] = new_field(1,10,0,0,0,0); // label id
    field_signup[1] = new_field(1,10,0,10,10,0); // id
    field_signup[2] = new_field(1,10,2,0,0,0); // label pass
    field_signup[3] = new_field(1,10,2,10,10,0); // pass
    field_signup[4] = new_field(1,10,4,0,0,0); // label pass confirm
    field_signup[5] = new_field(1,10,4,10,10,0); // pass confirm
    field_signup[6] = NULL;

    //mise en place des textes labels -- login
    set_field_buffer(field_login[0],0,"login:");
    set_field_buffer(field_login[2],0,"password:");

    //mise en place des textes labels -- signup
    set_field_buffer(field_signup[0],0,"login:");
    set_field_buffer(field_signup[2],0,"password:");
    set_field_buffer(field_signup[4],0,"confirm:");

    // CUSTOMISATION DES CHAMPS
    // label id
    set_field_opts(field_login[0], O_AUTOSKIP | O_PUBLIC | O_VISIBLE);
    // id
    set_field_opts(field_login[1], O_PUBLIC | O_VISIBLE | O_ACTIVE | O_EDIT);
    set_field_back(field_login[1], A_UNDERLINE);
    set_field_type(field_login[1], TYPE_ALNUM);
    //label pass
    set_field_opts(field_login[2], O_AUTOSKIP | O_PUBLIC | O_VISIBLE);
    // pass
    set_field_opts(field_login[3], O_VISIBLE | O_ACTIVE | O_EDIT);
    field_opts_off(field_login[3], O_PUBLIC);
    set_field_back(field_login[3], A_UNDERLINE);
    set_field_type(field_login[3], TYPE_ALNUM);

    //initialisation des champs signup
    // label id
    set_field_opts(field_signup[0], O_AUTOSKIP | O_PUBLIC | O_VISIBLE);
    // id
    set_field_opts(field_signup[1], O_PUBLIC | O_VISIBLE | O_ACTIVE | O_EDIT);
    set_field_back(field_signup[1], A_UNDERLINE);
    set_field_type(field_signup[1], TYPE_ALNUM);
    //label pass
    set_field_opts(field_signup[2], O_AUTOSKIP | O_PUBLIC | O_VISIBLE);
    // pass
    set_field_opts(field_signup[3], O_VISIBLE | O_ACTIVE | O_EDIT);
    field_opts_off(field_signup[3], O_PUBLIC);
    set_field_back(field_signup[3], A_UNDERLINE);
    set_field_type(field_signup[3], TYPE_ALNUM);
    //label pass confirm
    set_field_opts(field_signup[4], O_AUTOSKIP | O_PUBLIC | O_VISIBLE);
    // pass confirm
    set_field_opts(field_signup[5], O_VISIBLE | O_ACTIVE | O_EDIT);
    field_opts_off(field_signup[5], O_PUBLIC);
    set_field_back(field_signup[5], A_UNDERLINE);
    set_field_type(field_signup[5], TYPE_ALNUM);

    // creation du formulaire de connexion / login.
    form_login = new_form(field_login);
    form_signup = new_form(field_signup);

    // integration formulaires dans fenetres
    set_form_win(form_login, sub_win_login_panel);
    set_form_sub(form_login, derwin(sub_win_login_panel, 10, 55, 1, 1));
    set_form_win(form_signup, win_signup_panel);
    set_form_sub(form_login, derwin(win_signup_panel, 10, 55, 1, 1));

    // publication formulaire login + signup
    post_form(form_login);
    post_form(form_signup);

    //mvprintw(LINES - 2, 0, "Use UP or DOWN to switch between fields");
    set_current_field(form_login, field_login[0]);
    refresh();
    wrefresh(win_login_panel);
    wrefresh(sub_win_login_panel);
    wrefresh(win_signup_panel);
    wrefresh(sub_win_signup_panel);

    // insertion dans les panels
    // ordre des panels -> stdscr-0-1-2-3
    // panel 2 et 3 - login / signup

    panels[0] = new_panel(win_left_panel);
    panels[1] = new_panel(win_top_panel);
    panels[2] = new_panel(win_login_panel);
    panels[3] = new_panel(win_signup_panel);
    panels[4] = NULL;
    //mise a jour des panels pour finir la creation
    update_panels();
    // affichage sur ecran
    doupdate();
    //hide_panel(panels[3]);
    // boucle manipulation fenetre au clavier -- exercice, sert fondamentalement a rien;
    // directement copiee, j'ai deja fait le meme exo juste avant
    while((character = getch()) != KEY_F(4))
           driver_form(character, form_login, panels);

    // depubliage du formulaire
    unpost_form(form_login);
    //liberation memoire
    free_form(form_login);
    for (int i = 0; i < 4; ++i) {
        free_field(field_login[i]);
    }
    // fermeture fenetre
    delwin(win_left_panel);
    delwin(win_top_panel);
    delwin(win_login_panel);
    delwin(sub_win_login_panel);
    delwin(win_signup_panel);
    delwin(sub_win_signup_panel);
    endwin();
    return EXIT_SUCCESS;
}