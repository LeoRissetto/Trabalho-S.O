/*
Leonardo Gueno Rissetto 13676482
Lucas Lima Romero 13676325
Luciano Gonçalves Lopes Filho 13676520
Marco Antonio Gaspar Garcia 11833581
Thiago Kashivagui Gonçalves 13676579
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define TRUE 1
#define THREAD_NUM 5

sem_t semCanetaEmpty;
sem_t semCanetaFull;

pthread_mutex_t mutexMateria;

pthread_mutex_t mutexCaneta;

int maxStoreCaneta = 100; //5

int tempoEspera = 1; //7
int tempoEnvio = 2; //3
int tempoFazer = 3; //4

int qntCanetas = 0; //6
int qntEnviado = 0; //2

int qntMateria = 100; //1

//thread rank 1
void *depositoMateria(void *args){

    while(TRUE){

        sleep(tempoEnvio);        
    }
}

//thread rank 2
void *celulaFabricacao(void *args){

    while(TRUE){
        
        sleep(tempoFazer);
    }
}

//thread rank 3
void *controle(void *args){

    while(TRUE){

    }
}

//thread rank 4
void *depositoCanetas(void *args){

    while(TRUE){

        sem_wait(&semCanetaEmpty);
        pthread_mutex_lock(&mutexCaneta);
        qntCanetas++;
        pthread_mutex_unlock(&mutexCaneta);
        sem_post(&semCanetaFull);

        printf("Armazenou caneta\n");
    }
}

//thread rank 5
void *comprador(void *args){

    while(TRUE){

        sem_wait(&semCanetaFull);
        pthread_mutex_lock(&mutexCaneta);
        qntCanetas++;
        pthread_mutex_unlock(&mutexCaneta);
        sem_post(&semCanetaEmpty);

        printf("Consumiu caneta\n");

        sleep(tempoEspera);
    }
}

int main(int argc, char* argv[]){

    pthread_t th[THREAD_NUM];

    pthread_mutex_init(&mutexMateria, NULL);

    pthread_mutex_init(&mutexCaneta, NULL);

    sem_init(&semCanetaEmpty, 0, maxStoreCaneta);
    sem_init(&semCanetaFull, 0, 0);

    if(pthread_create(&th[1], NULL, &depositoMateria, NULL) != 0)
        perror("Falha ao criar thread\n");

    if(pthread_create(&th[2], NULL, &celulaFabricacao, NULL) != 0)
        perror("Falha ao criar thread\n");

    if(pthread_create(&th[3], NULL, &controle, NULL) != 0)
        perror("Falha ao criar thread\n");

    if(pthread_create(&th[4], NULL, &depositoCanetas, NULL) != 0)
        perror("Falha ao criar thread\n");

    if(pthread_create(&th[5], NULL, &comprador, NULL) != 0)
        perror("Falha ao criar thread\n");

    for(int i =0; i < THREAD_NUM; i++){
        if(pthread_join(th[i], NULL) != 0){
            perror("Falha ao dar join");
        }
    }

    sem_destroy(&semCanetaEmpty);
    sem_destroy(&semCanetaFull);

    pthread_mutex_destroy(&mutexMateria);

    pthread_mutex_destroy(&mutexCaneta);

    return 0;
}