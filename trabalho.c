#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define TRUE 1
#define THREAD_NUM 5

sem_t semEmpty;
sem_t semFull;

pthread_mutex_t mutexBuffer;

int buffer[10];
int count = 0;

int timeWait = 0;
int timeMake = 0;
int timeSend = 0;
int qntCanetas = 0;
int qntEnviado = 0;
int qntMateria = 0;
int maxStore = 0;

void *producer(void* args){

    while(TRUE){

        int x = rand() % 100;

        sleep(1);

        sem_wait(&semEmpty);
        pthread_mutex_lock(&mutexBuffer);
        buffer[count] = x;
        count++;
        pthread_mutex_unlock(&mutexBuffer);
        sem_post(&semFull);
    }
}

void *consumer(void *args){

    while(TRUE){

        int y;

        sem_wait(&semFull);
        pthread_mutex_lock(&mutexBuffer);
        y = buffer[count - 1];
        count--;
        pthread_mutex_unlock(&mutexBuffer);
        sem_post(&semEmpty);

        printf("Consumiu %d\n", y);

        sleep(1);
    }
}

//thread rank 1
void *depositoMateria(void *args){

    while(TRUE){

    }
}

//thread rank 2
void *celulaFabricacao(void *args){

    while(TRUE){

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

    }
}

//thread rank 5
void *comprador(void *args){

    while(TRUE){

    }
}

int main(){

    pthread_t th[THREAD_NUM];
    pthread_mutex_init(&mutexBuffer, NULL);

    sem_init(&semEmpty, 0, 10);
    sem_init(&semFull, 0, 0);

    int i = pthread_create(&th[1], NULL, &depositoMateria, NULL);
    i += pthread_create(&th[2], NULL, &celulaFabricacao, NULL);
    i += pthread_create(&th[3], NULL, &controle, NULL);
    i += pthread_create(&th[4], NULL, &depositoCanetas, NULL);
    i += pthread_create(&th[5], NULL, &comprador, NULL);

    if(i != 0){
        perror("Falha ao criar thread");
    }

    for(int i =0; i < THREAD_NUM; i++){
        if(pthread_join(th[i], NULL) != 0){
            perror("Falha ao dar join");
        }
    }

    sem_destroy(&semEmpty);
    sem_destroy(&semFull);

    pthread_mutex_destroy(&mutexBuffer);

    return 0;
}