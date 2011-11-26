#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "desenho.h"

//#define DEBUG

#define THREAD_NUM 100
#define CHANCE_DIR 0.5
#define MAX_CORDA 5
#define MAX_DIFF 10
#define MIN_CR_TIME 500
#define MAX_CR_TIME 1500
#define MIN_CREATION_TIME 100
#define MAX_CREATION_TIME 800

#define DIR 0
#define ESQ 1

#define DIR_ST 0
#define ESQ_ST 1
#define LIVRE_ST 2

#define IS_MY_DIRECTION(X) ( X == DIR && estado_corda == DIR_ST ) || ( X == ESQ && estado_corda == ESQ_ST )

#ifdef DEBUG
#define ANIMACAO(X) sem_wait(&animacao); X sem_post(&animacao);
#else
#define ANIMACAO(X) /**/

#endif

#define POST_V(S,X)  while (X--) sem_post(S);

#define MIN(A,B,C) if (A<B) C = A; else C = B;

#ifdef _LP64
typedef long long int bint;
#else
typedef long int  bint;
#endif

static sem_t esquerda;
static sem_t direita;
static sem_t corda;

#ifdef DEBUG
static sem_t animacao;
#endif

static volatile int estado_corda;
static volatile int nro_corda;
static volatile int balanco;
static volatile int esperando_dir;
static volatile int esperando_esq;

typedef struct
{
   unsigned short id;
   short sentido;
} info_t;

void muda_sentido(int sentido, int id)
{
   if (estado_corda == sentido)
      return;

   estado_corda = sentido;

   ANIMACAO
   (
      if (estado_corda == LIVRE_ST)
         printf("### LIVRE    : ESTADO_CORDA\n");
      else if (estado_corda == ESQ_ST)
         printf("### ESQUERDA : ESTADO_CORDA\n");
      else
         printf("### DIREITA  : ESTADO_CORDA\n");
   );

#ifndef DEBUG
   desenho_muda_corda(sentido, id);
#endif
}

int entra_corda(int s, int id)
{
   int p;
   int esperando;
   int pode_seguir;
   sem_t* sem;

#ifndef DEBUG
   desenho_tenta_corda(id);
   usleep(200000);
#endif

   sem_wait(&corda);

   if (estado_corda == LIVRE_ST)
   {
      // I'm the first to get to the rope, let's capture it
      if (s == DIR)
      {
         muda_sentido(DIR_ST, id);
         sem = &direita;
         esperando = esperando_dir;
      }
      else
      {
         muda_sentido(ESQ_ST, id);
         sem = &esquerda;
         esperando = esperando_esq;
      }

      // I can continue
      pode_seguir = 1;

      // and let my budies in, minus 1 because I'm already on the rope
      MIN(esperando, MAX_CORDA - 1, p);
      POST_V(sem, p);
      // note: esperando is always greather than 0, because esperando_[dir|esq]
      // is reduced only on the next if block
   }
   else if (IS_MY_DIRECTION(s))
   {
      pode_seguir = nro_corda < MAX_CORDA;
   }
   else
   {
      // oops! not my direction, I'll have to wait
      pode_seguir = 0;
   }
   // else: if rope's not free but everyone is going on my direction, let's keep on!

   ANIMACAO
   (
      if (s == DIR)
         printf("   <<< DIREITA  (%d): TENTA ENTRAR CORDA [BAL. %d; ESQ. %d; DIR: %d]\n", id, balanco, esperando_esq, esperando_dir);
      else
         printf("   <<< ESQUERDA (%d): TENTA ENTRAR CORDA [BAL. %d; ESQ. %d; DIR: %d]\n", id, balanco, esperando_esq, esperando_dir);
   );


   // if I can go on, let's let everyone know
   if (pode_seguir)
   {
      nro_corda++;

      ANIMACAO
      (
         if (s == DIR)
            printf("DIREITA  (%d): NA CORDA [BAL. %d; ESQ. %d; DIR: %d]\n", id, balanco, esperando_esq, esperando_dir);
         else
            printf("ESQUERDA (%d): NA CORDA [BAL. %d; ESQ. %d; DIR: %d]\n", id, balanco, esperando_esq, esperando_dir);
      );

      if (s == DIR)
      {
         balanco++;
         esperando_dir--;
      }
      else
      {
         balanco--;
         esperando_esq--;
      }
   }

   sem_post(&corda);

#ifndef DEBUG
   if (pode_seguir)
      desenho_entra_corda(id);
   else
      desenho_desiste_corda(id);
#endif

   return pode_seguir;
}

void sai_corda(int s, int id)
{
   int p;

#ifndef DEBUG
   desenho_sai_corda(id);
#endif

   sem_wait(&corda);
   
   nro_corda--;

   ANIMACAO
   (
      if (s == DIR)
         printf("      >>> DIREITA  (%d): SAI DA CORDA [BAL. %d; ESQ. %d; DIR: %d]\n", id, balanco, esperando_esq, esperando_dir);
      else
         printf("      >>> ESQUERDA (%d): SAI DA CORDA [BAL. %d; ESQ. %d; DIR: %d]\n", id, balanco, esperando_esq, esperando_dir);
   );

   // Am I the last one?
   if (nro_corda == 0)
   {   
      // if right side has used the rope too much and there're babuines on the left side waiting...
      if (balanco > MAX_DIFF && esperando_esq > 0)
      {
         // let left side play
         muda_sentido(ESQ_ST, id);
         MIN(esperando_esq, MAX_CORDA, p);
         POST_V(&esquerda, p);
      }
      // if left side has used the rope too much and there're babuines on the right side waiting...
      else if (balanco < -MAX_DIFF && esperando_dir > 0)
      {
         // let right side play
         muda_sentido(DIR_ST, id);
         MIN(esperando_dir, MAX_CORDA, p);
         POST_V(&direita, p);
      }
      else
      {
         // let anyone waiting play
         if (esperando_dir > 0)
         {
            // right
            muda_sentido(DIR_ST, id);
            MIN(esperando_dir, MAX_CORDA, p);
            POST_V(&direita, p);
         }
         else if (esperando_esq > 0)
         {
            // left
            muda_sentido(ESQ_ST, id);
            MIN(esperando_esq, MAX_CORDA, p);
            POST_V(&esquerda, p);
         }
         else
         {
            ANIMACAO(printf("@ULTIMO (%d): liberando corda\n", id);)
            // I'm the last one! It's my moral duty to free the rope
            muda_sentido(LIVRE_ST, id);

            sem_post(&esquerda);
            sem_post(&direita);
         }
      }
   }
   else
   {

      if ( (s == DIR && esperando_esq > 0 && balanco > MAX_DIFF) ||
           (s == ESQ && esperando_dir > 0 && balanco > -MAX_DIFF))
      {
         // well, we should let the other side play
      }
      else
      {
         // I'm not the last, just let one more in
         if (s == DIR)
            sem_post(&direita);
         else
            sem_post(&esquerda);
      } 
   }

   sem_post(&corda);
}

void do_work(int s)
{
   sem_wait(&corda);
/*
   ANIMACAO
   (
      if (s == DIR)
         printf("DIREITA  : NA CORDA [BAL. %d]\n", balanco);
      else
         printf("ESQUERDA : NA CORDA [BAL. %d]\n", balanco);
    );
*/
   sem_post(&corda);
   
   usleep(((random() % (MAX_CR_TIME - MIN_CR_TIME)) + MIN_CR_TIME)*1000);

}

void* babuino(void* info)
{
   // direction
   bint s;
   info_t in;
   int id;
   sem_t* sem;

   in = *((info_t*)&info);

   s = (bint)((int)in.sentido);



   if (s == ESQ)
      sem = &esquerda;
   else
      sem = &direita;

   sem_wait(&corda);
   #ifndef DEBUG
      id = desenho_novo_macaco(s);
   #else
      id = (int)in.id;
   #endif  
   
   if (s == DIR)
      esperando_dir++;
   else
      esperando_esq++;
      
   sem_post(&corda);   

   do
   {
      // I shouldn't overload the rope with my weight
      // wait for my budies to get to the other side
      sem_wait(sem);

      // let's see if I can get on the rope
   } while (!entra_corda(s, id));

   // ** yeap, on the rope! **
   do_work(s);

   // leaving the rope
   sai_corda(s, id);

   return NULL;
}

int main(int argn, char** argv)
{
   int i;
   pthread_t t[THREAD_NUM];
   info_t info;

#ifndef DEBUG
   desenho_init();
#endif

   // set random seed based on current time
   srandom((unsigned int)time(NULL));

   // init semaphores
   sem_init(&esquerda, 0, 1);
   sem_init(&direita, 0, 1);
   sem_init(&corda, 0, 1);

#ifdef DEBUG
   sem_init(&animacao, 0, 1);
#endif

   // set rope state
   estado_corda = LIVRE_ST;
   nro_corda = 0;
   balanco = 0;
   esperando_dir = 0;
   esperando_esq = 0;

   // create threads
   for (i = 0; i < THREAD_NUM; i++)
   {
      // babuino id
      info.id = (unsigned short)i;
      // random direction
      info.sentido = (short)(random() > RAND_MAX * CHANCE_DIR);
      usleep(((random() % (MAX_CREATION_TIME - MIN_CREATION_TIME)) + MIN_CREATION_TIME)*1000);
      pthread_create(&t[i], NULL, babuino, (int*)*((int*)&info));
   }

   // wait for threads to finish
   for (i = 0; i < THREAD_NUM; i++)
   {
      pthread_join(t[i], NULL);
   }

   sem_destroy(&esquerda);
   sem_destroy(&direita);
   sem_destroy(&corda);

#ifndef DEBUG
   desenho_destroy();
#endif

   return 0;
}
