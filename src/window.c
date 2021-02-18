// ncurses et ses extensions
#include <ncurses.h>
#include <panel.h>
#include <menu.h>
#include <form.h>

#include <stdio.h>
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

void init_window_params(WIN *param_win);
void create_box(WIN *win, bool flag);

int main(int argc, char *argv[]){
    // INIT du panel
    WINDOW *win[4];
    PANEL *panels[4];

    // initialialisation du formulaire de connexion
    FIELD *field_login[3];
    FORM *form_login;
    // init du form inscription
   /*
    FIELD *field_signup[4];
    FORM *form_signup;
    */
    // capture claver
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


    // initialisation des champs
    // login
    for (int i = 0; i < 2; ++i) {
        field_login[i] = new_field(1,10,3+i,18,0,0);
        field_opts_off(field_login[i], O_AUTOSKIP);
    }
    field_login[2] = NULL;
    // id
    set_field_fore(field_login[0],COLOR_PAIR(1));
    set_field_back(field_login[0],COLOR_PAIR(2));
    set_field_type(field_login[0], TYPE_ALNUM);
    // pass
    field_opts_off(field_login[1], O_PUBLIC);
    set_field_fore(field_login[1],A_UNDERLINE);
    set_field_back(field_login[1],A_UNDERLINE);
    // signup
   /* for (int i = 0; i < 3; ++i) {
        field_signup[i] = new_field(1,10,4,18,0,0);
        set_field_back(field_signup[i], A_BOLD);
        field_opts_off(field_signup[i], O_AUTOSKIP);
    }
    field_signup[3] = NULL;
    */
    // creation formulaire
    form_login = new_form(field_login);
    //form_signup = new_form(field_signup);

    // publication formulaire
    post_form(form_login);
    //post_form(form_signup);

    // insersion dans la fenetre stdscr
    refresh();

    // texte pour faire joli - tutoriel

    mvprintw(LINES - 2, 0, "Use UP or DOWN to switch between fields");
    set_current_field(form_login, field_login[0]);
    refresh();

    // creation des windows avant entree dans les panels

    win[0] = newwin(LINES, RATIOX, 0, 0);
    win[1] = newwin(RATIOY, COLS, 0, 0);
    win[2] = newwin(10, 20, LINES - 10 / 2, COLS - 5);
    win[3] = newwin(10, 20, LINES - 10 / 2, COLS - 5);

    mvwprintw(win[2], 4, 5, "login: ");
    mvwprintw(win[2], 6, 5, "pass : ");

    mvwprintw(win[3], 4, 5,"new login   : ");
    mvwprintw(win[3], 5, 5,"new pass    : ");
    mvwprintw(win[3], 6, 5,"confirm pass: ");

    // box autour des panels
    for(int i = 0; i < 4; ++i) {
        box(win[i], 0, 0);
    }

    // insertion dans les panels
    // ordre des panels -> stdscr-0-1-2-3
    // panel 2 et 3 - login / signup
    for (int i = 0; i < 4; ++i) {
        panels[i] = new_panel(win[i]);
    }

    //mise a jour des panels pour finir la creation
    update_panels();
    // affichage sur ecran
    doupdate();
/*
    // initialisation des fenetres
    init_window_params(&win);

    // attributs des fenetres :
    attron(COLOR_PAIR(1));
    printw("press F4 to exit");
    refresh();
    attroff(COLOR_PAIR(1));

    // creation de la window
    create_box(&win, TRUE);
*/
    // boucle manipulation fenetre au clavier -- exercice, sert fondamentalement a rien;
    // directement copiee, j'ai deja fait le meme exo juste avant
    while((character = getch()) != KEY_F(4))
    {
        switch(character)
        {
            case KEY_LEFT:
                break;
            case KEY_RIGHT:
                break;
            case KEY_UP:
                form_driver(form_login, REQ_PREV_FIELD);
                form_driver(form_login, REQ_END_LINE);
                break;
            case KEY_DOWN:
                form_driver(form_login, REQ_NEXT_FIELD);
                form_driver(form_login, REQ_END_LINE);
                break;
            default:
                form_driver(form_login, character);
                break;
        }
    }
    // depubliage du formulaire
    unpost_form(form_login);

    for (int i = 0; i < 2; ++i) {
        free_field(field_login[i]);
    }
    // fermeture fenetre
    endwin();
    return EXIT_SUCCESS;
}

void init_window_params(WIN *param_win){
    // initialisation de la taille + centre terminal
    // option ++ on peux meme eventuellement faire un ratio ecran

    param_win->height = 5;
    param_win->width = 10;
    param_win->starty = (LINES - param_win->height) / 2;
    param_win->startx = (COLS - param_win->width) / 2;

    // init des bordures pour faire un joli contour
    param_win->border.left = '|';
    param_win->border.right = '|';
    param_win->border.top = '-';
    param_win->border.bottom = '-';
    param_win->border.topLeft = '+';
    param_win->border.topRight = '+';
    param_win->border.bottomLeft = '+';
    param_win->border.bottomRight = '+';
}
/*
void create_box(WIN *param_win, bool flag){
    int i, j;
    int x, y, winHeight, winWidth;

    x = param_win->startx;
    y = param_win->starty;
    winWidth = param_win->width;
    winHeight = param_win->height;

    if (flag == TRUE){
        mvaddch(y, x, param_win->border.topLeft);
        mvaddch(y, x + winWidth, param_win->border.topRight);
        mvaddch(y + winHeight, x, param_win->border.bottomLeft);
        mvaddch(y + winHeight, x + winWidth, param_win->border.bottomRight);
        mvhline(y, x + 1, param_win->border.top, winWidth- 1);
        mvhline(y + winHeight, x + 1, param_win->border.bottom, winWidth - 1);
        mvvline(y + 1, x, param_win->border.left, winHeight -1);
        mvvline(y + 1, x + winWidth, param_win->border.right, winHeight -1);
    } else {
        for (j = y; j <= y + winHeight; ++j) {
            for (i = x; i <= x + winWidth; ++i) {
                mvaddch(j, i , ' ');
            }
        }
    }
    refresh();
} */