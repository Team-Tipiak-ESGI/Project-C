#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    // chtype est un type exclusif a ncurses, il definit une variable comme etant un
    // character, un attribut ncurses, et un colour-pair, c'est a dire : gestion couleur.
    chtype left;
    chtype right;
    chtype top;
    chtype bottom;
    chtype topLeft;
    chtype topRight;
    chtype bottomLeft;
    chtype bottomRight;
} WIN_BORDER;

typedef struct {

    int startx;
    int starty;
    int width;
    int height;
    WIN_BORDER border;
} WIN;

void init_window_params(WIN *param_win);
void print_window_params(WIN *param_win);
void create_box(WIN *win, bool flag);

int main(int argc, char *argv[]){
    WIN win;
    int character;
    //init ecran et fenetres
    initscr();
    if (has_colors() == FALSE){
    } else {
        start_color(); // couleurs
    }
    cbreak(); //pas de line buffering, tout sera passe au programme
    keypad(stdscr, TRUE); // touches fonctions
    noecho(); // pas de texte au clavier quand on fait des input;
    init_pair(1, COLOR_CYAN, COLOR_BLACK); // couleurs sur le terminal, texte cyan, fond noir

    // initialisation des fenetres
    init_window_params(&win);
    print_window_params(&win);

    // attributs des fenetres :
    attron(COLOR_PAIR(1));
    printw("press F4 to exit");
    refresh();
    attroff(COLOR_PAIR(1));

    // creation de la window
    create_box(&win, TRUE);
    // boucle manipulation fenetre au clavier -- exercice, sert fondamentalement a rien;
    // directement copiee, j'ai deja fait le meme exo juste avant
    while((character = getch()) != KEY_F(4))
    {
        switch(character)
        {
            case KEY_LEFT:
                create_box(&win, FALSE);
                --win.startx;
                create_box(&win, TRUE);
                break;
            case KEY_RIGHT:
                create_box(&win, FALSE);
                ++win.startx;
                create_box(&win, TRUE);
                break;
            case KEY_UP:
                create_box(&win, FALSE);
                --win.starty;
                create_box(&win, TRUE);
                break;
            case KEY_DOWN:
                create_box(&win, FALSE);
                ++win.starty;
                create_box(&win, TRUE);
                break;
        }
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

void print_window_params(WIN *param_win){
#ifdef _DEBUG
    mvprintw(25, 0, "%d %d %d %d", param_win->startx, param_win->starty, param_win->width, param_win->height);
#endif
}
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
}