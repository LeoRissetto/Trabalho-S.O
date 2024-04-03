#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define TRUE 1
#define THREAD_NUM 2

sem_t semEmpty;
sem_t semFull;

pthread_mutex_t mutexBuffer;

int buffer[10];
int count = 0;

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

int main(){

    pthread_t th[THREAD_NUM];
    pthread_mutex_init(&mutexBuffer, NULL);

    sem_init(&semEmpty, 0, 10);
    sem_init(&semFull, 0, 0);

    for(int i =0; i < THREAD_NUM; i++){
        if(i  % 2 == 0){
            if (pthread_create(&th[i], NULL, &producer, NULL) != 0){
            perror("Falha ao criar a thread");
            }
        }
        else{
            if (pthread_create(&th[i], NULL, &consumer, NULL) != 0){
                perror("Falha ao criar a thread");
            }
        }
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