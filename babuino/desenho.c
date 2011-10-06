#include "desenho.h"

#define _COR(C,X) attron(COLOR_PAIR(C)); X attroff(COLOR_PAIR(C));
#define BLACK(X) _COR(0,X)
#define RED(X) _COR(1,X)
#define GREEN(X) _COR(2,X)
#define YELLOW(X) _COR(3,X)
#define BLUE(X) _COR(4,X)
#define MARGENTA(X) _COR(5,X)
#define CYAN(X) _COR(6,X)
#define WHITE(X) _COR(7,X)

#define LOCK(X) pthread_mutex_lock(&lock); X pthread_mutex_unlock(&lock);

#define D_SLEEP usleep(500000)

static mapa_t mapa;
static pthread_mutex_t lock;

static volatile macaco_t* macacos;

// posicoes ocupadas em cada local
static volatile pos_t *esquerda;
static volatile pos_t *direita;
static volatile pos_t *corda;
static volatile pos_t *tenta_corda_dir;
static volatile pos_t *tenta_corda_esq;
static volatile pos_t *saiu_corda_dir;
static volatile pos_t *saiu_corda_esq;

void free_l(volatile pos_t* p)
{   
   pos_t* temp;
   
   while(p != NULL)
   {
      temp = p;
      p = p->next;
      free(temp);
   }
}

void free_m(volatile macaco_t* p)
{
   pos_t* temp;
   
   while(p != NULL)
   {
      temp = p;
      p = p->next;
      free(temp);
   }
}

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

      D_SLEEP;
   )
}

void desenha_mapa()
{
   int col, row;
   int i;
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
   //esquerda
   m->esq_ix = 3;
   m->esq_iy = m->ponte_y + 3;
   m->esq_fx = m->ponte_x - 3;
   
   // direita
   m->dir_ix = m->ponte_x + m->ponte_len + 3;
   m->dir_iy = m->ponte_y + 3;
   m->dir_fx = col - 3;

   // espera esq
   m->esp_cor_esq_ix = 0;
   m->esp_cor_esq_iy = m->ponte_y - 1;
   m->esp_cor_esq_fx = m->ponte_x - 1;

   // espera dir
   m->esp_cor_dir_ix = m->ponte_x + m->ponte_len + 1;
   m->esp_cor_dir_iy = m->ponte_y - 1;
   m->esp_cor_dir_fx = col;

   // corda
   m->cor_ix = m->ponte_x;
   m->cor_iy = m->ponte_y - 1;
   m->cor_fx = m->ponte_x + m->ponte_len;

   // saida esq
   m->saiu_esq_ix = m->esq_ix;
   m->saiu_esq_iy = m->esq_iy + 10;
   m->saiu_esq_fx = m->esq_fx;
   
   // direita
   m->saiu_dir_ix = m->dir_ix;
   m->saiu_dir_iy = m->dir_iy + 10;
   m->saiu_dir_fx = m->dir_fx;

   // placa
   m->placa_x = m->ponte_x + m->ponte_len/2;
   for (i = 0; i < altura_placa - 2; i++)
         mvprintw(i, m->placa_x - 3, "|         |");


   mvprintw(altura_placa - 2, m->placa_x - 3, "###########");
   mvprintw(altura_placa - 1, m->placa_x - 3, "#  LIVRE  #");
   mvprintw(altura_placa - 0, m->placa_x - 3, "###########");

   // posicao do texto da placa
   m->placa_x = m->placa_x;
   m->placa_y = altura_placa - 1;

   // macaco grande
   desenha_macaco_grde(5, 6);
   desenha_macaco_grde(col - 17, 6);

   refresh();
}

void desenho_init()
{
   pthread_mutexattr_t mattr;
   // init ncurses
   initscr();

   // init recursive lock
   pthread_mutexattr_init(&mattr);
   pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE_NP);
   pthread_mutex_init(&lock, &mattr);
   pthread_mutexattr_destroy(&mattr);

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
   saiu_corda_dir = NULL;
   saiu_corda_esq = NULL;

   desenha_mapa();
}

void desenho_destroy()
{
   // destroy lock
   pthread_mutex_destroy(&lock);

   // wait for user to finish
   getch();

   // finish ncurses
   endwin();

   free_m(macacos);
   free_l(esquerda);
   free_l(direita);
   free_l(corda);
   free_l(tenta_corda_esq);
   free_l(tenta_corda_dir);
   free_l(saiu_corda_dir);
   free_l(saiu_corda_dir);
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
         i = 1;
         for (n = (macaco_t*)macacos; n->next != NULL; n = n->next)
            i++;

         n->next = m;
      }

      m->id = i;
   )

   return m;
}

/* deve ser chamado com lock */
void libera_pos(int x, int y, pos_t** pos)
{
   pos_t* p, *prev;

   if ((*pos)->x == x && (*pos)->y == y)
   {
      p = *pos;
      *pos = (*pos)->next;
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

   return m->id;
}

void desenho_muda_corda(int novo_sentido, int id_macaco)
{
   char *livre, *esq, *dir, *v;

   livre = "LIVRE";
   esq   = "<<---";
   dir   = "--->>";

   if (novo_sentido == DIREITA)
      v = esq;
   else if (novo_sentido == ESQUERDA)
      v = dir;
   else
      v = livre;

   LOCK
   (
      mvprintw(mapa.placa_y, mapa.placa_x, v);
      refresh();
      D_SLEEP;
   )
}

void desenho_entra_corda(int id_macaco)
{
   // macaco estava tentando entrar na corda
   macaco_t *m;
   pos_t** p;
   pos_t* pc;

   LOCK
   (
      m = get_macaco(id_macaco);

      if (m->estado == ESP_CORDA_DIR)
      {
         p = (pos_t**)&tenta_corda_dir;
         m->estado = CORDA_DIR;
      }
      else
      {
         p = (pos_t**)&tenta_corda_esq;
         m->estado = CORDA_ESQ;
      }
     
      // libera pos de tentativa corda
      libera_pos(m->x, m->y, p);

      // pega posicao na corda
      pc = aloca_pos((pos_t**)&corda, mapa.cor_ix, mapa.cor_iy, mapa.cor_fx);
      
      // move macaco
      move_macaco(m, pc->x, pc->y);
   )
}

void desenho_tenta_corda(int id_macaco)
{
   macaco_t *m;
   pos_t **p, **pe;
   pos_t* pc;
   int ix, iy, fx;

   LOCK
   (
      m = get_macaco(id_macaco);

      if (m->estado == DIREITA)
      {
         p = (pos_t**)&direita;
         pe = (pos_t**)&tenta_corda_dir;
         m->estado = ESP_CORDA_DIR;
         ix = mapa.esp_cor_dir_ix;
         iy = mapa.esp_cor_dir_iy;
         fx = mapa.esp_cor_dir_fx;
      }
      else
      {
         p = (pos_t**)&esquerda;
         pe = (pos_t**)&tenta_corda_esq;
         m->estado = ESP_CORDA_ESQ;
         ix = mapa.esp_cor_esq_ix;
         iy = mapa.esp_cor_esq_iy;
         fx = mapa.esp_cor_esq_fx;
      }
     
      // libera pos anterior
      libera_pos(m->x, m->y, p);

      // pega posicao na espera da corda
      pc = aloca_pos(pe, ix, iy, fx);
      
      // move macaco
      move_macaco(m, pc->x, pc->y);
   )
}

void desenho_desiste_corda(int id_macaco)
{
   macaco_t *m;
   pos_t **p, **pe;
   pos_t* pc;
   int ix, iy, fx;

   LOCK
   (
      m = get_macaco(id_macaco);

      if (m->estado == ESP_CORDA_DIR)
      {
         pe = (pos_t**)&direita;
         p = (pos_t**)&tenta_corda_dir;
         m->estado = DIREITA;
         ix = mapa.dir_ix;
         iy = mapa.dir_iy;
         fx = mapa.dir_fx;
      }
      else
      {
         pe = (pos_t**)&esquerda;
         p = (pos_t**)&tenta_corda_esq;
         m->estado = ESQUERDA;
         ix = mapa.esq_ix;
         iy = mapa.esq_iy;
         fx = mapa.esq_fx;
      }
     
      // libera pos anterior
      libera_pos(m->x, m->y, p);

      // pega posicao na espera
      pc = aloca_pos(pe, ix, iy, fx);
      
      // move macaco
      move_macaco(m, pc->x, pc->y);
   )
}

void desenho_sai_corda(int id_macaco)
{
   macaco_t *m;
   pos_t **p, **pe;
   pos_t* pc;
   int ix, iy, fx;

   LOCK
   (
      m = get_macaco(id_macaco);

      p = (pos_t**)&corda;

      if (m->estado == CORDA_DIR)
      {
         pe = (pos_t**)&saiu_corda_esq;
         m->estado = SAIU_DIR;
         ix = mapa.saiu_esq_ix;
         iy = mapa.saiu_esq_iy;
         fx = mapa.saiu_esq_fx;
      }
      else
      {
         pe = (pos_t**)&saiu_corda_dir;
         m->estado = SAIU_ESQ;
         ix = mapa.saiu_dir_ix;
         iy = mapa.saiu_dir_iy;
         fx = mapa.saiu_dir_fx;
      }
     
      // libera pos anterior
      libera_pos(m->x, m->y, p);

      // pega posicao na saida
      pc = aloca_pos(pe, ix, iy, fx);
      
      // move macaco
      move_macaco(m, pc->x, pc->y);
   )
}
