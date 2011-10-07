#ifndef __DESENHO__
#define __DESENHO__

#include <curses.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define DIREITA          0
#define ESQUERDA         1
#define ESP_CORDA_DIR    2
#define ESP_CORDA_ESQ    3
#define CORDA_DIR        4
#define CORDA_ESQ        5
#define SAIU_DIR         6
#define SAIU_ESQ         7

#define BLACK	0
#define RED		1
#define GREEN	2
#define YELLOW	3
#define BLUE	4
#define MAGENTA	5
#define CYAN	6
#define WHITE	7

typedef struct mapa
{
   int ponte_x, ponte_y, ponte_len;
   int placa_x, placa_y;

   // posicoes de inicio e fim do retangulo de espera da esquerda
   // e da direita
   int esq_ix, esq_iy, esq_fx;
   int dir_ix, dir_iy, dir_fx;

   // posicao de inicio e fim da corda
   int cor_ix, cor_iy, cor_fx;

   // posicoes de espera
   int esp_cor_dir_ix, esp_cor_dir_iy, esp_cor_dir_fx;
   int esp_cor_esq_ix, esp_cor_esq_iy, esp_cor_esq_fx;

   // posicao saida
   int saiu_dir_ix, saiu_dir_iy, saiu_dir_fx;
   int saiu_esq_ix, saiu_esq_iy, saiu_esq_fx;

} mapa_t;

typedef struct pos
{
   int x;
   int y;
   struct pos *next;
} pos_t;

typedef struct macaco
{
   int id;
   int sentido;
   int x, y;
   int estado;
   int color;
   struct macaco* next;
} macaco_t;

void desenho_init();

void desenho_destroy();

void desenho_mapa();

/* cria novo macaco e retorna id do macaco
 tambem desenha macaco no lado certo */
int desenho_novo_macaco(int sentido);

/* muda sentido da corda */
void desenho_muda_corda(int novo_sentido, int id_macaco);

/* informa que macaco entrou na corda (desenho_tenta_corda deve ser chamado antes) */
void desenho_entra_corda(int id_macaco);

/* informa que macaco saiu da corda */
void desenho_sai_corda(int id_macaco);

/* informa que macaco tentou pegar a corda */
void desenho_tenta_corda(int id_macaco);

/* informa que macaco desistiu da corda */
void desenho_desiste_corda(int id_macaco);

#endif
