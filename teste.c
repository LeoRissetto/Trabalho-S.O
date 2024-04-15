#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_CANETAS 5
#define TRUE 1

// Definição das estruturas de dados
typedef struct {
    int material;
    int materialEnviado;
    sem_t mutex;
    sem_t empty; // Semáforo para indicar se o depósito de material está vazio
    sem_t full;  // Semáforo para indicar se o depósito de material está cheio
} DepositoMaterial;

typedef struct {
    int canetas;
    int canetasEnviadas;
    sem_t mutex;
    sem_t empty; // Semáforo para indicar se o depósito de canetas está vazio
    sem_t full;  // Semáforo para indicar se o depósito de canetas está cheio
} DepositoCaneta;

// Variáveis globais
DepositoMaterial depositoMaterial;
DepositoCaneta depositoCaneta;

sem_t fabrica;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

// Funções auxiliares
void *deposito_material(){

    int qntEnviada = 1;
    int tempoEnvio = 1;

    while (TRUE) { 
        sem_wait(&depositoCaneta.empty);

        sem_wait(&depositoMaterial.empty);
        sem_wait(&depositoMaterial.mutex);

        if(depositoMaterial.material == 0){
            qntEnviada = 0;
            printf("Depósito de Material: Acabou a matéria-prima.\n");
        }
        else{
            if(depositoMaterial.material < qntEnviada){
                qntEnviada = depositoMaterial.material;
            }
            depositoMaterial.material -= qntEnviada;
            depositoMaterial.materialEnviado += qntEnviada;
            printf("Depósito de Material: Enviando %d unidades de matéria-prima.\n", qntEnviada);
        }

        sem_post(&depositoMaterial.mutex);

        for(int i = 0; i < qntEnviada; i++){
            sem_post(&depositoMaterial.full);
        }

        sleep(tempoEnvio);
    }

    return NULL;
}

void *fabrica_caneta(){

    int tempoFabricacao = 1;

    while (TRUE) {
        sem_wait(&depositoCaneta.empty);

        sem_wait(&depositoMaterial.full);

        sem_wait(&depositoMaterial.mutex);
        sem_wait(&depositoCaneta.mutex);

        depositoMaterial.materialEnviado--;
        depositoCaneta.canetas++;
        printf("Célula de fabricação de canetas: fabricou 1 caneta. Estoque: %d\n", depositoMaterial.materialEnviado);

        sem_post(&depositoCaneta.mutex);
        sem_post(&depositoMaterial.mutex);

        pthread_cond_signal(&condition);

        sleep(tempoFabricacao);
    }

    return NULL;
}

void *controle(){

    while(TRUE) {

    }

    return NULL;
}

void *deposito_caneta(){

    int tempoEnvio = 1;
    int qntEnviada;

    while (TRUE) {
        sem_wait(&depositoCaneta.empty);
        sem_wait(&depositoCaneta.mutex);

        pthread_mutex_lock(&mutex);
        while(depositoCaneta.canetas == 0){
            pthread_cond_wait(&condition, &mutex);
        }
        pthread_mutex_unlock(&mutex);

        if(depositoCaneta.canetasEnviadas + depositoCaneta.canetas < MAX_CANETAS){
            qntEnviada = depositoCaneta.canetas;
        }
        else{
            qntEnviada = MAX_CANETAS - depositoCaneta.canetasEnviadas; 
        }

        depositoCaneta.canetas -= qntEnviada;
        depositoCaneta.canetasEnviadas += qntEnviada;
        printf("Depósito de Canetas: Enviadas %d canetas. Estoque: %d\n", qntEnviada, depositoCaneta.canetasEnviadas);

        sem_post(&depositoCaneta.mutex);

        for(int i = 0; i < qntEnviada; i++){
            sem_post(&depositoCaneta.full);
        }

        sleep(tempoEnvio);
    }

    return NULL;
}

void *comprador(){

    int tempoEspera = 1;
    int qntComprada = 2;

    while (TRUE) {
        sem_wait(&depositoCaneta.full);
        sem_wait(&depositoCaneta.mutex);

        if(depositoCaneta.canetasEnviadas < qntComprada){
            qntComprada = depositoCaneta.canetasEnviadas;
        }
        depositoCaneta.canetasEnviadas -= qntComprada;
        printf("Comprador: Comprou %d canetas.\n", qntComprada);

        sem_post(&depositoCaneta.mutex);

        for(int i = 0; i < qntComprada; i++){
            sem_post(&depositoCaneta.empty);
        }

        sleep(tempoEspera);
    }

    return NULL;
}

int main(int argc, char *argv[]){
    // Inicializando as variáveis e semáforos
    depositoMaterial.material = 10;
    depositoMaterial.materialEnviado = 0;
    sem_init(&depositoMaterial.mutex, 0, 1);
    sem_init(&depositoMaterial.empty, 0, depositoMaterial.material);
    sem_init(&depositoMaterial.full, 0, 0);

    depositoCaneta.canetas = 0;
    depositoCaneta.canetasEnviadas = 0;
    sem_init(&depositoCaneta.mutex, 0, 1);
    sem_init(&depositoCaneta.empty, 0, MAX_CANETAS);
    sem_init(&depositoCaneta.full, 0, 0);

    sem_init(&fabrica, 0, 0);

    // Inicialização das threads
    pthread_t threads[4];
    pthread_create(&threads[0], NULL, deposito_material, NULL);
    pthread_create(&threads[1], NULL, fabrica_caneta, NULL);
    pthread_create(&threads[2], NULL, deposito_caneta, NULL);
    pthread_create(&threads[3], NULL, comprador, NULL);

    // Aguardar o término das threads
    for (int i = 0; i < 4; ++i) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
