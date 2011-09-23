#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define THREAD_NUM 100
#define CHANCE_DIR 0.5
#define MAX_CORDA 5

#define DIR 0
#define ESQ 1

#define DIR_ST 0
#define ESQ_ST 1
#define LIVRE_ST 2

#define IS_MY_DIRECTION(X) ( X == DIR && estado_corda == DIR_ST ) || ( X == ESQ && estado_corda == ESQ_ST )

#define ANIMACAO(X) sem_wait(&animacao); X sem_post(&animacao);


sem_t esquerda;
sem_t direita;
sem_t corda;
sem_t animacao;

volatile int estado_corda;
volatile int nro_corda;

int entra_corda(int s)
{
   int i;
   int pode_seguir;
   sem_t* sem;

   sem_wait(&corda);

   if (estado_corda == LIVRE_ST)
   {
      ANIMACAO
      (
         printf("Primeiro: liberando corda para ");

         if (s == DIR)
            printf("direita\n");
         else
            printf("esquerda\n");
      )

      // I'm the first to get to the rope, let's capture it
      if (s == DIR)
      {
         estado_corda = DIR_ST;
         sem = &direita;
      }
      else
      {
         estado_corda = ESQ_ST;
         sem = &esquerda;
      }

      // I can continue
      pode_seguir = 1;

      // and let my budies in
      for (i = 0; i < MAX_CORDA - 1; i++)
         sem_post(sem);
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

   // if I can go on, let's let everyone know
   if (pode_seguir)
      nro_corda++;

   sem_post(&corda);

   return pode_seguir;
}

void sai_corda(int s)
{
   int i;

   sem_wait(&corda);

   nro_corda--;

   if (nro_corda == 0)
   {     
/*
      sem_getvalue(&esquerda, &i);

      if (i < MAX_CORDA
*/

      ANIMACAO(printf("Ultimo: liberando corda\n");)

      // I'm the last one! It's my moral duty to free the rope
      estado_corda = LIVRE_ST;

      sem_post(&esquerda);
      sem_post(&direita);
   }


   sem_post(&corda);
}

void print(int s)
{
   ANIMACAO
   (

         if (estado_corda == LIVRE_ST)
            printf("corda livre - ");
         else if (estado_corda == ESQ_ST)
            printf("corda esquerda - ");
         else
            printf("corda direita - ");

         if (s == DIR)
            printf("babuino direita\n");
         else
            printf("babuino esquerda\n");

    )
}

void* babuino(void* sen)
{
   // direction
   int s;
   int entrou_corda;
   sem_t* sem;

   s = (int)(long long int)sen;
   entrou_corda = 0;

   if (s == ESQ)
      sem = &esquerda;
   else
      sem = &direita;

   // I shouldn't overload the rope with my weight
   // wait for my budies to get to the other side
   sem_wait(sem);

   // let's see if I can get on the rope
   while (!entra_corda(s))
   {
      // well, not this time, I'll wait here
      sem_wait(sem);
   } 


   // ** yeap, on the rope! **
   print(s);

   // leaving the rope
   sai_corda(s);

   return NULL;
}

int main(int argn, char** argv)
{
   int i;
   int s;
   pthread_t t[THREAD_NUM];

   // set random seed based on current time
   srandom((unsigned int)time(NULL));

   // init semaphores
   sem_init(&esquerda, 0, 1);
   sem_init(&direita, 0, 1);
   sem_init(&corda, 0, 1);
   sem_init(&animacao, 0, 1);

   // set rope state
   estado_corda = LIVRE_ST;
   nro_corda = 0;

   // create threads
   for (i = 0; i < THREAD_NUM; i++)
   {
      // random direction
      s = random() > RAND_MAX * CHANCE_DIR;
      pthread_create(&t[i], NULL, babuino, (int*)(long long int)s);
   }

   // wait for threads to finish
   for (i = 0; i < THREAD_NUM; i++)
   {
      pthread_join(t[i], NULL);
   }

   sem_destroy(&esquerda);
   sem_destroy(&direita);
   sem_destroy(&corda);

   return 0;
}
