#include "desenho.h"

#define _COR(C,X) attron(COLOR_PAIR(C)); X attroff(COLOR_PAIR(C));
#define BRANCO(X) _COR(1,X)
#define BRANCO(X) _COR(1,X)
#define BRANCO(X) _COR(1,X)

#define LOCK(X) sem_wait(&lock); X sem_post(&lock);
#define FREE_L(X) for (; X != NULL; X = X = X->next) free((void*)X);

#define D_SLEEP 1

static mapa_t mapa;
static sem_t lock;

static volatile macaco_t* macacos;

// posicoes ocupadas em cada local
static volatile pos_t *esquerda;
static volatile pos_t *direita;
static volatile pos_t *corda;
static volatile pos_t *tenta_corda_dir;
static volatile pos_t *tenta_corda_esq;


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

static void move_macaco(macaco_t* m, int x, int y)
{
   char* d;

   if (m->sentido == DIREITA)
      d = "<";
   else
      d = ">";

   LOCK
   (
      // apaga posicao anterior
      mvprintw(m->y, m->x, " ");
      
      m->x = x;
      m->y = y;
      
      // desenha posicao nova
      mvprintw(m->y, m->x, d);

      refresh();

      sleep(D_SLEEP);
   )
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

   // define limites
   m->esq_ix = 3;
   m->esq_iy = m->ponte_y + 3;
   m->esq_fx = m->ponte_x - 3;

   m->dir_ix = m->ponte_x + m->ponte_len + 3;
   m->dir_iy = m->ponte_y + 3;
   m->dir_fx = col - 3;

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

   refresh();
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
   esquerda = NULL;
   direita = NULL;
   corda = NULL;
   tenta_corda_esq = NULL;
   tenta_corda_dir = NULL;

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

   FREE_L(macacos);
   FREE_L(esquerda);
   FREE_L(direita);
   FREE_L(corda);
   FREE_L(tenta_corda_esq);
   FREE_L(tenta_corda_dir);
}

macaco_t* new_macaco(int sentido)
{
   int i;
   macaco_t* m, *n;
   i = 0;

   m = (macaco_t*)malloc(sizeof(macaco_t));
   m->sentido = sentido;
   m->estado = sentido;
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

void libera_pos(int x, int y, pos_t** pos)
{
   pos_t* p, *prev;

   LOCK
   (
      if ((*pos)->x == x && (*pos)->y == y)
      {
         p = *pos;
         *pos = NULL;
      }
      else
      {
         for (p = (*pos)->next, prev = *pos; p != NULL; p = p->next)
         {
            if (p->x == x && p->y == y)
            {
               prev->next = p->next;
               break;
            }
            
            prev = p;
         }
      }
   )

   free(p);
}

pos_t* ocupa_pos(int x, int y, pos_t** pos)
{
   pos_t* p, *n;

   p = (pos_t*)malloc(sizeof(pos_t));
   p->x = x;
   p->y = y;
   p->next = NULL;

   if (!*pos)
      *pos = p;
   else
   {
      for (n = *pos; n->next != NULL; n = n->next);
      n->next = p;
   }
   
   return p;
}

pos_t* aloca_pos(pos_t** pos, int ix, int iy, int fx)
{
   int x, y;
   int diff;
   pos_t* p;

   diff = fx - ix + 1;
   
   LOCK
   (
      p = *pos;
      
      for (x = 0, y = 0; p != NULL && p->x == ix + x && p->y == iy + y; x = (x + 1) % diff)
      {
         p = p->next;
         if (x == diff - 1)
            y++;
      }
      
      p = ocupa_pos(ix + x, iy + y, pos);
   )

   return p;
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
   macaco_t* m;
   pos_t* p;
   m = new_macaco(sentido);

   if (sentido == ESQUERDA)
      p = aloca_pos((pos_t**)&esquerda, mapa.esq_ix, mapa.esq_iy, mapa.esq_fx);
   else
      p = aloca_pos((pos_t**)&direita, mapa.dir_ix, mapa.dir_iy, mapa.dir_fx);

   move_macaco(m, p->x, p->y);
}

void desenho_muda_corda(int novo_sentido, int id_macaco)
{
   
}
