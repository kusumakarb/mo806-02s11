#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "desenho.h"

#define THREAD_NUM 100
#define CHANCE_DIR 0.5
#define MAX_CORDA 5
#define MAX_DIFF 10

#define DIR 0
#define ESQ 1

#define DIR_ST 0
#define ESQ_ST 1
#define LIVRE_ST 2

#define IS_MY_DIRECTION(X) ( X == DIR && estado_corda == DIR_ST ) || ( X == ESQ && estado_corda == ESQ_ST )

#define ANIMACAO(X) sem_wait(&animacao); X sem_post(&animacao);

#define POST_V(S,X)  while (X--) sem_post(S);

#define MIN(A,B,C) if (A<B) C = A; else C = B;

sem_t esquerda;
sem_t direita;
sem_t corda;
sem_t animacao;

volatile int estado_corda;
volatile int nro_corda;
volatile int balanco;
volatile int esperando_dir;
volatile int esperando_esq;

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
   {
      nro_corda++;

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

   return pode_seguir;
}

void sai_corda(int s)
{
   int p;

   sem_wait(&corda);

   nro_corda--;

   // Am I the last one?
   if (nro_corda == 0)
   {   
      ANIMACAO(printf("Ultimo\nEsp_dir: %d\nEsp_esq: %d\nBalanco: %d\n", esperando_dir, esperando_esq, balanco);)

      // if right side has used the rope too much and there're babuines on the left side waiting...
      if (balanco > MAX_DIFF && esperando_esq > 0)
      {
         // let left side play
         estado_corda = ESQ_ST;
         MIN(esperando_esq, MAX_CORDA, p);
         POST_V(&esquerda, p);
      }
      // if left side has used the rope too much and there're babuines on the right side waiting...
      else if (balanco < -MAX_DIFF && esperando_dir > 0)
      {
         // let right side play
         estado_corda = DIR_ST;
         MIN(esperando_dir, MAX_CORDA, p);
         POST_V(&direita, p);
      }
      else
      {
         // let anyone waiting play
         if (esperando_dir > 0)
         {
            // right
            estado_corda = DIR_ST;
            MIN(esperando_dir, MAX_CORDA, p);
            POST_V(&direita, p);
         }
         else if (esperando_esq > 0)
         {
            // left
            estado_corda = ESQ_ST;
            MIN(esperando_esq, MAX_CORDA, p);
            POST_V(&esquerda, p);
         }
         else
         {
            // I'm the last one! It's my moral duty to free the rope
            estado_corda = LIVRE_ST;
            
            sem_post(&esquerda);
            sem_post(&direita);

            ANIMACAO(printf("Ultimo: liberando corda\n");)
         }
      }
   }
   else
   {
      // I'm not the last, just let one more in
      if (s == DIR)
         sem_post(&direita);
      else
         sem_post(&esquerda);
   }

   sem_post(&corda);
}

void print(int s)
{
   sem_wait(&corda);

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

    sem_post(&corda);
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

   sem_wait(&corda);
   if (s == DIR)
      esperando_dir++;
   else
      esperando_esq++;
   sem_post(&corda);   

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

   desenho_init();

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
   balanco = 0;
   esperando_dir = 0;
   esperando_esq = 0;

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

   desenho_destroy();

   return 0;
}
