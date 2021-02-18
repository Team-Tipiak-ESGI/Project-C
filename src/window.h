//
// Created by simon on 17/02/2021.
//

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


// definition de la taille de la fenetre
// et inclusion des bordures dans la fenetre
typedef struct {

    int startx;
    int starty;
    int width;
    int height;
    WIN_BORDER border;
} WIN;