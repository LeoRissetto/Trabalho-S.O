#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_CANETAS 5
#define TRUE 1

typedef struct {
    int material;
    sem_t mutex;
    sem_t pode_produzir;
    sem_t pode_enviar;
} DepositoMaterial;

typedef struct {
    int canetas;
    sem_t mutex;
    sem_t pode_armazenar;
    sem_t pode_vender;
} DepositoCaneta;

DepositoMaterial depositoMaterial;
DepositoCaneta depositoCaneta;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

void *deposito_material(void *arg) {
    int qntEnviada = 1;
    while (TRUE) {
        sem_wait(&depositoMaterial.pode_enviar);
        sem_wait(&depositoMaterial.mutex);

        if (depositoMaterial.material > 0) {
            depositoMaterial.material -= qntEnviada;
            pthread_mutex_lock(&print_mutex);
            printf("Depósito de Material: Enviando %d unidades de matéria-prima. Restante: %d\n", qntEnviada, depositoMaterial.material);
            pthread_mutex_unlock(&print_mutex);
            sem_post(&depositoMaterial.pode_produzir);
        }

        sem_post(&depositoMaterial.mutex);
        sleep(1);
    }
    return NULL;
}

void *fabrica_caneta(void *arg) {
    while (TRUE) {
        sem_wait(&depositoMaterial.pode_produzir);
        sem_wait(&depositoCaneta.pode_armazenar);

        sem_wait(&depositoCaneta.mutex);

        depositoCaneta.canetas++;
        pthread_mutex_lock(&print_mutex);
        printf("Célula de fabricação de canetas: Fabricou 1 caneta. Total disponível: %d\n", depositoCaneta.canetas);
        pthread_mutex_unlock(&print_mutex);

        sem_post(&depositoCaneta.mutex);
        sem_post(&depositoCaneta.pode_vender);

        sleep(1);
    }
    return NULL;
}

void *controle(void *arg) {
    while(TRUE) {
        sem_wait(&depositoCaneta.pode_armazenar);
        if (depositoCaneta.canetas < MAX_CANETAS) {
            sem_post(&depositoMaterial.pode_enviar);
        }
        sem_post(&depositoCaneta.pode_armazenar);
    }
    return NULL;
}

void *deposito_caneta(void *arg) {
    while (TRUE) {
        sem_wait(&depositoCaneta.pode_vender);
        sem_wait(&depositoCaneta.mutex);

        depositoCaneta.canetas--;
        pthread_mutex_lock(&print_mutex);
        printf("Depósito de Canetas: Enviou 1 caneta. Canetas restantes: %d\n", depositoCaneta.canetas);
        pthread_mutex_unlock(&print_mutex);

        sem_post(&depositoCaneta.mutex);
        sem_post(&depositoCaneta.pode_armazenar);
        sleep(1);
    }
    return NULL;
}

void *comprador(void *arg) {
    while (TRUE) {
        sem_wait(&depositoCaneta.pode_vender);
        sem_wait(&depositoCaneta.mutex);

        depositoCaneta.canetas--;
        pthread_mutex_lock(&print_mutex);
        printf("Comprador: Comprou 1 caneta. Canetas restantes: %d\n", depositoCaneta.canetas);
        pthread_mutex_unlock(&print_mutex);

        sem_post(&depositoCaneta.mutex);
        sem_post(&depositoCaneta.pode_armazenar);
        sleep(1);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    depositoMaterial.material = 10;

    sem_init(&depositoMaterial.mutex, 0, 1);
    sem_init(&depositoMaterial.pode_produzir, 0, 0);
    sem_init(&depositoMaterial.pode_enviar, 0, 1);

    sem_init(&depositoCaneta.mutex, 0, 1);
    sem_init(&depositoCaneta.pode_armazenar, 0, MAX_CANETAS);
    sem_init(&depositoCaneta.pode_vender, 0, 0);

    pthread_t threads[6];
    pthread_create(&threads[0], NULL, deposito_material, NULL);
    pthread_create(&threads[1], NULL, fabrica_caneta, NULL);
    pthread_create(&threads[2], NULL, controle, NULL);
    pthread_create(&threads[3], NULL, deposito_caneta, NULL);
    pthread_create(&threads[4], NULL, comprador, NULL);

    for (int i = 0; i < 5; ++i) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
