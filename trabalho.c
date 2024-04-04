#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define TRUE 1
#define THREAD_NUM 5

sem_t semMateriaEmpty;
sem_t semMateriaFull;

sem_t semCanetaEmpty;
sem_t semCanetaFull;

pthread_mutex_t mutexMateria;

pthread_mutex_t mutexCaneta;

int maxStoreMateria = 100;
int maxStoreCaneta = 100;

int tempoEspera = 0;
int tempoEnvio = 0;
int tempoFazer = 0;
int qntCanetas = 0;
int qntEnviado = 0;
int qntMateria = 0;

//thread rank 1
void *depositoMateria(void *args){

    while(TRUE){

        sem_wait(&semMateriaEmpty);
        pthread_mutex_lock(&mutexMateria);
        qntMateria--;
        pthread_mutex_unlock(&mutexMateria);
        sem_post(&semMateriaFull);

        printf("Enviou materia prima\n");

        sleep(tempoEnvio);
    }
}

//thread rank 2
void *celulaFabricacao(void *args){

    while(TRUE){

        sem_wait(&semMateriaFull);
        pthread_mutex_lock(&mutexMateria);
        // Fabricar canetas
        pthread_mutex_unlock(&mutexMateria);
        sem_post(&semMateriaEmpty);

        printf("Produziu caneta\n");

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

    sem_init(&semMateriaEmpty, 0, maxStoreMateria);
    sem_init(&semMateriaFull, 0, 0);

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

    sem_destroy(&semMateriaEmpty);
    sem_destroy(&semMateriaFull);

    sem_destroy(&semCanetaEmpty);
    sem_destroy(&semCanetaFull);

    pthread_mutex_destroy(&mutexMateria);

    pthread_mutex_destroy(&mutexCaneta);

    return 0;
}