#ifndef __DESENHO__
#define __DESENHO__

#include <curses.h>
#include <string.h>
#include <semaphore.h>
#include <stdlib.h>

#define DIREITA          0
#define ESQUERDA         1
#define ESPERANDO_CORDA  2
#define CORDA            3

typedef struct mapa
{
   int ponte_x, ponte_y, ponte_len;
   int placa_x, placa_y;
} mapa_t;

typedef struct macaco
{
   int id;
   int sentido;
   int x, y;
   int estado;
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
