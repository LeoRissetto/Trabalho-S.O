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
    pthread_mutex_t mutex;
    sem_t empty; // Semáforo para indicar se o depósito de material está vazio
    sem_t full;  // Semáforo para indicar se o depósito de material está cheio
} DepositoMaterial;

typedef struct {
    int canetas;
    int canetasEnviadas;
    pthread_mutex_t mutex;
    sem_t empty; // Semáforo para indicar se o depósito de canetas está vazio
    sem_t full;  // Semáforo para indicar se o depósito de canetas está cheio
} DepositoCaneta;

// Variáveis globais
DepositoMaterial depositoMaterial;
DepositoCaneta depositoCaneta;

sem_t mutex;

// Funções auxiliares
void *deposito_material(){

    int qntEnviada = 5;
    int tempoEnvio = 3;

    while (TRUE) { 
        sem_wait(&depositoCaneta.empty);

        sem_wait(&depositoMaterial.empty);
        pthread_mutex_lock(&depositoMaterial.mutex);

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

        pthread_mutex_unlock(&depositoMaterial.mutex);

        for(int i = 0; i < qntEnviada; i++){
            sem_post(&depositoMaterial.full);
        }

        sem_post(&depositoCaneta.empty);

        sleep(tempoEnvio);
    }

    return NULL;
}

void *fabrica_caneta(){

    int tempoFabricacao = 1;

    while (TRUE) {
        sem_wait(&depositoCaneta.empty);

        sem_wait(&depositoMaterial.full);
        pthread_mutex_lock(&depositoMaterial.mutex);

        pthread_mutex_lock(&depositoCaneta.mutex);

        int x = depositoCaneta.canetas;

        depositoMaterial.materialEnviado--;
        depositoCaneta.canetas++;
        printf("Célula de fabricação de canetas: fabricou 1 caneta. Estoque de Material: %d\n", depositoMaterial.materialEnviado);

        pthread_mutex_unlock(&depositoCaneta.mutex);
        pthread_mutex_unlock(&depositoMaterial.mutex);

        if(x == 0){
            sem_post(&mutex);
        }

        sem_post(&depositoCaneta.empty);

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

    int tempoEnvio = 2;
    int qntEnviada;

    while (TRUE) {
        sem_wait(&mutex);

        sem_wait(&depositoCaneta.empty);
        pthread_mutex_lock(&depositoCaneta.mutex);

        if(depositoCaneta.canetasEnviadas + depositoCaneta.canetas < MAX_CANETAS){
            qntEnviada = depositoCaneta.canetas;
        }
        else{
            qntEnviada = MAX_CANETAS - depositoCaneta.canetasEnviadas;
            sem_post(&mutex);
        }

        depositoCaneta.canetas -= qntEnviada;
        depositoCaneta.canetasEnviadas += qntEnviada;
        printf("Depósito de Canetas: Enviadas %d canetas. Estoque: %d\n", qntEnviada, depositoCaneta.canetasEnviadas);

        pthread_mutex_unlock(&depositoCaneta.mutex);

        for(int i = 0; i < qntEnviada; i++){
            sem_post(&depositoCaneta.full);
        }

        sleep(tempoEnvio);
    }

    return NULL;
}

void *comprador(){

    int tempoEspera = 5;
    int qntComprada = 3;

    while (TRUE) {
        sem_wait(&depositoCaneta.full);
        pthread_mutex_lock(&depositoCaneta.mutex);

        if(qntComprada > depositoCaneta.canetasEnviadas){
            qntComprada = depositoCaneta.canetasEnviadas;
        }
        else{
            qntComprada = 3;
        }
        depositoCaneta.canetasEnviadas -= qntComprada;
        printf("Comprador: Comprou %d canetas.\n", qntComprada);

        pthread_mutex_unlock(&depositoCaneta.mutex);

        for(int i = 0; i < qntComprada; i++){
            sem_post(&depositoCaneta.empty);
        }

        sleep(tempoEspera);
    }

    return NULL;
}

void *encerrar(){

    while(TRUE){
        pthread_mutex_lock(&depositoMaterial.mutex);
        pthread_mutex_lock(&depositoCaneta.mutex);
        if(depositoMaterial.material == 0 && depositoCaneta.canetas == 0 &&
         depositoCaneta.canetasEnviadas == 0 && depositoMaterial.materialEnviado == 0){
            exit(0);
        }
        pthread_mutex_unlock(&depositoMaterial.mutex);
        pthread_mutex_unlock(&depositoCaneta.mutex);
    }

    return NULL;
}

int main(int argc, char *argv[]){
    // Inicializando as variáveis e semáforos
    depositoMaterial.material = 28;
    depositoMaterial.materialEnviado = 0;
    pthread_mutex_init(&depositoMaterial.mutex, NULL);
    sem_init(&depositoMaterial.empty, 0, depositoMaterial.material);
    sem_init(&depositoMaterial.full, 0, 0);

    depositoCaneta.canetas = 0;
    depositoCaneta.canetasEnviadas = 0;
    pthread_mutex_init(&depositoCaneta.mutex, NULL);
    sem_init(&depositoCaneta.empty, 0, MAX_CANETAS);
    sem_init(&depositoCaneta.full, 0, 0);

    sem_init(&mutex, 0, 0);

    // Inicialização das threads
    pthread_t threads[5];
    pthread_create(&threads[0], NULL, deposito_material, NULL);
    pthread_create(&threads[1], NULL, fabrica_caneta, NULL);
    pthread_create(&threads[2], NULL, deposito_caneta, NULL);
    pthread_create(&threads[3], NULL, comprador, NULL);
    pthread_create(&threads[4], NULL, encerrar, NULL);

    // Aguardar o término das threads
    for (int i = 0; i < 5; ++i) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
