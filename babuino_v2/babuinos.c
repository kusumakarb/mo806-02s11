#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define THREAD_NUM 100
#define CHANCE_DIR 0.5
#define LOCK(X) sem_wait(&lock); X sem_post(&lock);

// Semaforos
static sem_t direita;
static sem_t esquerda;
static sem_t lock;

// Define do sentido da corda
#define DIR_ST 0
#define ESQ_ST 1
#define LIVRE_ST 2

static volatile int id = 0;
static volatile int estado_corda;

// numero de babuinos na corda
static volatile int num_babuinos_na_corda = 0;
//numero de babuinos esperando na esquerda
static volatile int num_babuinos_esq_wait = 0;
//numero de babuinos esperando na direita
static volatile int num_babuinos_dir_wait = 0;
//numero de babuinos vindo da esquerda saindo da corda
static volatile int num_babuinos_esq_exit = 0;
//numero de babuinos vindo da direita saindo da corda
static volatile int num_babuinos_dir_exit = 0;

int entra_corda(int sentido)
{
   int id_macaco;
   
   LOCK
   (
      // Atribui o valor de ID para o macaco
      id_macaco = id++;
   
      // Incrementa o numero de babuinos esperando
      if(sentido == DIR_ST)
         num_babuinos_dir_wait++;
      else
         num_babuinos_esq_wait++;
      
      // Quando o estado da corda está livre, inicializa o valor do semaforo
      if(estado_corda == LIVRE_ST)
      {
         if(sentido == DIR_ST)
         {           
            sem_init(&direita, 0, 5);
            sem_init(&esquerda, 0, 0);
            estado_corda = DIR_ST;
            printf("### ESTADO DA CORDA: direita\n");
         }
         else
         {
            sem_init(&esquerda, 0, 5);
            sem_init(&direita, 0, 0);
            printf("### ESTADO DA CORDA: esquerda\n");
            estado_corda = ESQ_ST;
         }
      }
      
      if(sentido == DIR_ST)
         printf("   <<< DIREITA ESPERANDO NA CORDA: %d\n", id);
      else
         printf("   <<< ESQUERDA ESPERANDO NA CORDA: %d\n", id);
   )
   return id_macaco;
}

void na_corda(int sentido, int id)
{
   
   if(sentido == DIR_ST)
   {
      sem_wait(&direita);
      num_babuinos_dir_wait--;
   }
   else
   {
      sem_wait(&esquerda);
      num_babuinos_esq_wait--;
   }
   
   LOCK(
      num_babuinos_na_corda++;
      sleep(random()%3);
      if(sentido == DIR_ST)
         printf("DIREITA NA CORDA: %d\n", id);
      else
         printf("ESQUERDA NA CORDA: %d\n", id);
   )

   if(sentido == DIR_ST)
   {
      sem_post(&direita);
   }
   else
   {
      sem_post(&esquerda);
   }
}

void sai_corda(int sentido, int id)
{
   int i;
   LOCK
   (
      if(sentido == DIR_ST)
         printf("      >>> DIREITA SAINDO DA CORDA: %d\n", id);
      else
         printf("      >>> ESQUERDA SAINDO CORDA: %d\n", id);
      
      num_babuinos_na_corda--;
      
      if(num_babuinos_na_corda == 0){
         // Quando nao tem nenhum babuino esperando dos dois lados
         if(num_babuinos_dir_wait == 0 && num_babuinos_esq_wait == 0)
         {
            estado_corda = LIVRE_ST;
            printf("### ESTADO DA CORDA: livre\n");
         }
         else if(sentido == DIR_ST)
         {
            if(num_babuinos_dir_wait == 0)
            {
               estado_corda = ESQ_ST;
               printf("### ESTADO DA CORDA: esquerda\n");
               for(i=0; i < 5; i++)
               {
                  if(num_babuinos_esq_wait > 0)
                     sem_post(&esquerda);
               }
            }
            else
            {
               for(i=0; i < 5; i++)
               {
                  if(num_babuinos_dir_wait > 0)
                     sem_post(&direita);
               }
            }
         }
         else
         {
            if(num_babuinos_esq_wait == 0)
            {
               estado_corda = DIR_ST;
               printf("### ESTADO DA CORDA: direita\n");
               for(i=0; i < 5; i++)
               {
                  if(num_babuinos_dir_wait > 0)
                     sem_post(&direita);
               }
            }
            else
            {
               for(i=0; i < 5; i++)
               {
                  if(num_babuinos_esq_wait > 0)
                     sem_post(&esquerda);
               }
            }
         }
      }
   
      if(sentido == DIR_ST)
      {
         num_babuinos_dir_exit++;
      }
      else
      {
         num_babuinos_esq_exit++;
      }
   )
}

void* babuino(void* v){
   int sentido = (int) v;
   int id;
 
   id = entra_corda(sentido); 
   na_corda(sentido, id);
   sai_corda(sentido, id);
    
   return NULL;
}

int main(int argn, char** argv)
{
   pthread_t t[THREAD_NUM];
   int s;
   int i;
   
   // init semaphores
   sem_init(&lock, 0, 1);

   estado_corda = LIVRE_ST;

   // create threads
   for (i = 0; i < THREAD_NUM; i++)
   {
      // random direction
      s = random() > RAND_MAX * CHANCE_DIR;
      sleep(random()%2);
      pthread_create(&t[i], NULL, babuino, (int*)s);
   }

   // wait for threads to finish
   for (i = 0; i < THREAD_NUM; i++)
   {
      pthread_join(t[i], NULL);
   }

   sem_destroy(&direita);
   sem_destroy(&esquerda);
   sem_destroy(&lock);

   return 0;
}
