#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_CANETAS 5
#define QNT_COMPRADA 1
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

// Funções auxiliares
void *deposito_material(){

    int qntEnviada = 1;
    int tempoEnvio = 2;

    while (TRUE) {
        sem_wait(&depositoMaterial.mutex);
        // Verifica se há material disponível antes de enviar
        sem_wait(&depositoCaneta.empty);
        depositoMaterial.material -= qntEnviada;
        depositoMaterial.materialEnviado += qntEnviada;
        printf("Depósito de Material: Enviando %d unidades de matéria-prima.\n", qntEnviada);
        for(int i = 0; i < qntEnviada; i++) {
            sem_wait(&depositoMaterial.empty);
            sem_post(&depositoMaterial.full);
        }
        sem_post(&depositoMaterial.mutex);
        sleep(tempoEnvio);
    }

    return NULL;
}

void *fabrica_caneta(){

    int tempoFabricacao = 2;

    while (TRUE) {
        sem_wait(&depositoMaterial.mutex);
        sem_wait(&depositoCaneta.mutex);
        // Verifica se há material disponível antes de iniciar a produção de canetas
        sem_wait(&depositoCaneta.empty);
        sem_wait(&depositoMaterial.full);

        depositoMaterial.materialEnviado--;
        depositoCaneta.canetas++;
        printf("Célula de fabricação de canetas: fabricou 1 caneta. Estoque: %d\n", depositoMaterial.materialEnviado);

        sem_post(&fabrica);
        
        sem_post(&depositoCaneta.mutex);
        sem_post(&depositoMaterial.mutex);
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
    
    int qntEnviada = 1;
    int tempoEnvio = 2;

    while (TRUE) {
        sem_wait(&depositoCaneta.mutex);
        sem_wait(&depositoCaneta.empty);
        sem_wait(&fabrica);
        // Verifica se há canetas disponíveis antes de enviar
        depositoCaneta.canetasEnviadas += qntEnviada;
        depositoCaneta.canetas -= qntEnviada;
        printf("Depósito de Canetas: Enviadas %d canetas. Estoque: %d\n", qntEnviada, depositoCaneta.canetasEnviadas);
        sem_post(&depositoCaneta.full);
        sem_post(&depositoCaneta.mutex);
        sleep(tempoEnvio);
    }

    return NULL;
}

void *comprador(){

    int tempoEspera = 2;

    while (TRUE) {
        sem_wait(&depositoCaneta.mutex);
        // Verifica se há canetas disponíveis para compra
        sem_wait(&depositoCaneta.full);
        depositoCaneta.canetasEnviadas -= QNT_COMPRADA;
        printf("Comprador: Comprou %d canetas.\n", QNT_COMPRADA);
        sem_post(&depositoCaneta.full);
        sem_post(&depositoCaneta.mutex);
        sleep(tempoEspera);
    }

    return NULL;
}

int main(int argc, char *argv[]){
    // Inicializando as variáveis e semáforos
    depositoMaterial.material = 10;
    depositoMaterial.materialEnviado = 0;
    depositoCaneta.canetas = 0;
    depositoCaneta.canetasEnviadas = 0;

    sem_init(&depositoMaterial.mutex, 0, 1);
    sem_init(&depositoMaterial.empty, 0, depositoMaterial.material);
    sem_init(&depositoMaterial.full, 0, 0);

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
