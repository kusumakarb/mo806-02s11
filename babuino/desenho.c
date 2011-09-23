#include "desenho.h"

mapa_t m;

void desenha_mapa()
{
   int col, row;
   int i, j;
   int distancia_topo = 10;
   char* ponte = "--------------------------------------------------------";

   getmaxyx(stdscr,row,col);

   m.ponte_x = ( col - strlen(ponte) ) /2;
   m.ponte_y = distancia_topo;
   m.ponte_len = strlen(ponte);

   // ponte
   mvprintw(m.ponte_y, m.ponte_x, ponte);

   // altura
   for (i = m.ponte_y + 1; i < row; i++)
   {
      mvprintw(i, m.ponte_x, "|");
      mvprintw(i,m.ponte_x + m.ponte_len - 1, "|");
   }

   // chao esq
   for (i = m.ponte_x - 1; i >= 0; i--)
      mvprintw(m.ponte_y, i, "_");

   // chao dir
   for (i = m.ponte_x + m.ponte_len + 1; i < col; i++)
      mvprintw(m.ponte_y, i, "_");

}

void desenho_init()
{
   // init ncurses
   initscr();

   // invisible cursor
   curs_set(0);

   desenha_mapa();
}

void desenho_destroy()
{
   // wait for user to finish
   getch();

   // finish ncurses
   endwin();
}
