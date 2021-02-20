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

// definition hauteur / largeur par defaut
#define WIDTH 30
#define HEIGHT 10
// ratio pour les panels
#define RATIOY (LINES * 0.2)
#define RATIOX (COLS * 0.1)

//ncurses apprecie pas de generer des
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
    //WINDOW *win_signup_panel, *sub_win_signup_panel;
    PANEL *panels[7];

    // initialialisation du formulaire de connexion
    FIELD *field_login[5];
    FORM *form_login;

    // init du form inscription
    /*
     FIELD *field_signup[7];
     FORM *form_signup;
     */

    // creation des windows avant entree dans les panels
    win_left_panel = newwin(LINES, RATIOX, 0, 0); // left panel
    win_top_panel = newwin(RATIOY, COLS, 0, 0); // top panel
    win_login_panel = newwin(10, 50, (LINES - 10)/ 2, (COLS - 50)/2); // login_panel // body
    sub_win_login_panel = derwin(win_login_panel, LINES / 2, COLS / 2, 3, 1); // sub_panel // form
    //win_signup_panel = newwin(10, 50, (LINES - 10 )/ 2, (COLS - 5)/2); // password_panel
    //sub_win_signup_panel = derwin(win_signup_panel,6,48,(LINES - 10 )/ 2, (COLS - 5)/2);

    mvwprintw(win_login_panel, 1, 2, "Press F4 to quit - F2 to switch between login / sign up");

    // box autour des win_*_panels
    box(win_top_panel, 0, 0);
    box(win_left_panel, 0, 0);
    box(win_login_panel, 0, 0);
    box(sub_win_login_panel, 0, 0);
    //box(win_signup_panel, 0, 0);
    //box(sub_win_signup_panel, 0, 0);

    // initialisation des champs de login
    field_login[0] = new_field(1,10,0,0,0,0); // label id
    field_login[1] = new_field(1,15,0,15,0,0); // id
    field_login[2] = new_field(1,10,2,0,0,0); // label pass
    field_login[3] = new_field(1,15,2,15,0,0); // pass
    field_login[4] = NULL;

    //mise en place des textes labels
    set_field_buffer(field_login[0],0,"login:");
    set_field_buffer(field_login[2],0,"password:");

    // CUSTOMISATION DES CHAMPS
    // label id
    set_field_opts(field_login[0], O_AUTOSKIP | O_PUBLIC | O_VISIBLE);
    // id
    set_field_opts(field_login[1], O_PUBLIC | O_VISIBLE | O_ACTIVE | O_EDIT);
    set_field_back(field_login[1],A_UNDERLINE);
    set_field_type(field_login[1], TYPE_ALNUM);
    //label pass
    set_field_opts(field_login[2], O_AUTOSKIP | O_PUBLIC | O_VISIBLE);
    // pass
    set_field_opts(field_login[3], O_VISIBLE | O_ACTIVE | O_EDIT);
    field_opts_off(field_login[3], O_PUBLIC);
    set_field_back(field_login[3],A_UNDERLINE);
    set_field_type(field_login[3], TYPE_ALNUM);

    //initialisation des champs signup
    //label id
    //id
    //label pass
    //pass
    //label confirm pass
    //confirm pass

    // creation du formulaire de connexion / login.
    form_login = new_form(field_login);
    //form_signup = new_form(field_signup);

    // integration formulaires dans fenetres
    set_form_win(form_login, sub_win_login_panel);
    set_form_sub(form_login, derwin(sub_win_login_panel, 18, 76, 3, 1));
    //set_form_win(form_signup, win_signup_panel);
    //set_form_sub(form_login, derwin(win_signup_panel, 18, 76, 1, 1));

    // publication formulaire login + signup
    post_form(form_login);
//  post_form(form_signup);

    //mvprintw(LINES - 2, 0, "Use UP or DOWN to switch between fields");
    //set_current_field(form_login, field_login[0]);
    refresh();
    wrefresh(sub_win_login_panel);
    wrefresh(win_login_panel);
    //wrefresh(sub_win_login_panel);


    // insertion dans les panels
    // ordre des panels -> stdscr-0-1-2-3
    // panel 2 et 3 - login / signup

    panels[0] = new_panel(win_left_panel);
    panels[1] = new_panel(win_top_panel);
    panels[2] = NULL; new_panel(win_login_panel);
    panels[3] = NULL; new_panel(sub_win_login_panel);
    //panels[4] = NULL;

    //mise a jour des panels pour finir la creation
    update_panels();
    // affichage sur ecran
    doupdate();

    // boucle manipulation fenetre au clavier -- exercice, sert fondamentalement a rien;
    // directement copiee, j'ai deja fait le meme exo juste avant
    while((character = getch()) != KEY_F(4))
    {
        switch(character)
        {
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
            case 127:
                form_driver(form_login, REQ_DEL_PREV);
                break;
            // Supprime le caractere qui est sur le curseur
            case KEY_DC:
                form_driver(form_login, REQ_DEL_CHAR);
                break;
            default:
                form_driver(form_login, character);
                break;
            // Remonter au debut ou a la fin
            case KEY_HOME:
                form_driver(form_login, REQ_BEG_LINE);
                break;
            case KEY_END:
                form_driver(form_login, REQ_END_LINE);
                break;
        }
    }
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
    //delwin(win_signup_panel);
    //delwin(sub_win_signup_panel);
    endwin();
    return EXIT_SUCCESS;
}
