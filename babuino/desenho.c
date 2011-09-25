#include "desenho.h"

#define _COR(C,X) attron(COLOR_PAIR(C)); X attroff(COLOR_PAIR(C));
#define BRANCO(X) _COR(1,X)
#define BRANCO(X) _COR(1,X)
#define BRANCO(X) _COR(1,X)

#define LOCK(X) sem_wait(&lock); X sem_post(&lock);

mapa_t mapa;
sem_t lock;

volatile macaco_t* macacos;

void desenha_macaco_grde(int x, int y)
{
/*
         .-"-.
       _/.-.-.\_
      ( ( o o ) )
       |/  "  \|
        \  O  /
        /`"""`\
       /       \
*/

   mvprintw(y-6, x, "   .-\"-.");
   mvprintw(y-5, x, " _/.-.-.\\_");
   mvprintw(y-4, x, "( ( o o ) )");
   mvprintw(y-3, x, " |/  \"  \\|");
   mvprintw(y-2, x, "  \\  O  /");
   mvprintw(y-1, x, "  /`\"\"\"`\\");
   mvprintw(y-0, x, " /       \\");
}

void desenha_macaco(macaco_t* m)
{
   char* d;

   if (m->sentido == DIREITA)
      d = ">";
   else
      d = "<";

   mvprintw(m->y, m->x, d);
}

void desenha_mapa()
{
   int col, row;
   int i, j;
   mapa_t* m;
   int distancia_topo, altura_placa;
   char* ponte;

   m = &mapa;
   distancia_topo = 20;
   altura_placa = 10;
   ponte = "--------------------------------------------------------";




   getmaxyx(stdscr,row,col);

   m->ponte_x = ( col - strlen(ponte) ) /2;
   m->ponte_y = distancia_topo;
   m->ponte_len = strlen(ponte);

   // ponte
   mvprintw(m->ponte_y, m->ponte_x, ponte);

   // altura
   for (i = m->ponte_y + 1; i < row; i++)
   {
      mvprintw(i, m->ponte_x, "|");
      mvprintw(i, m->ponte_x + m->ponte_len - 1, "|");
   }

   // chao esq
   for (i = m->ponte_x - 1; i >= 0; i--)
      mvprintw(m->ponte_y, i, "_");

   // chao dir
   for (i = m->ponte_x + m->ponte_len + 1; i < col; i++)
      mvprintw(m->ponte_y, i, "_");

   // placa
   m->placa_x = m->ponte_x + m->ponte_len/2;
   for (i = 0; i < altura_placa - 2; i++)
         mvprintw(i, m->placa_x - 3, "|         |");

   mvprintw(altura_placa - 2, m->placa_x - 3, "###########");
   mvprintw(altura_placa - 1, m->placa_x - 3, "#  LIVRE  #");
   mvprintw(altura_placa - 0, m->placa_x - 3, "###########");

   // posicao do texto da placa
   m->placa_x = m->placa_x - 3;
   m->placa_y = altura_placa - 1;

   // macaco grande
   desenha_macaco_grde(5, 6);
   desenha_macaco_grde(col - 17, 6);
}

void desenho_init()
{
   // init ncurses
   initscr();

   // init lock
   sem_init(&lock, 0, 1);

   // invisible cursor
   curs_set(0);

   start_color();

   init_pair(1, COLOR_WHITE, COLOR_BLACK);

   macacos = NULL;

   desenha_mapa();
}

void desenho_destroy()
{
   // destroy lock
   sem_destroy(&lock);

   // wait for user to finish
   getch();

   // finish ncurses
   endwin();

   for (; macacos != NULL; macacos = macacos = macacos->next)
      free((void*)macacos);

   macacos = NULL;
}

macaco_t* new_macaco(int sentido)
{
   int i;
   macaco_t* m, *n;
   i = 0;

   m = (macaco_t*)malloc(sizeof(macaco_t));
   m->sentido = sentido;
   m->next = NULL;

   LOCK
   (
      if (macacos == NULL)
         macacos = m;
      else
      {
         for (n = (macaco_t*)macacos; n->next != NULL; n = n->next)
            i++;

         n->next = m;
      }

      m->id = i;
   )

   return m;
}

macaco_t* get_macaco(int id)
{
   macaco_t* m;

   LOCK
   (
      m = (macaco_t*)macacos;
      while (id--) m = m->next;
   )

   return m;
}

int desenho_novo_macaco(int sentido)
{
   new_macaco(sentido);
}
