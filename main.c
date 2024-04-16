#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_CANETAS 2
#define TRUE 1

// Definição das estruturas de dados
typedef struct {
    int materialDeposito; // Quantidade de naterial no deposito
    int materialFabrica; // Quantidade de material na fabrica
    pthread_mutex_t mutex; // Mutex para alterar as variaveis (zona critica)
    sem_t full;  // Semaforo para indicar a quantidade de material na fabrica
} DepositoMaterial;

typedef struct {
    int canetasFabrica; // Quantidade de canetas na fabrica
    int canetasDeposito; // Quantidade de canetas no deposito
    pthread_mutex_t mutex; // Mutex para alterar as variaveis (zona critica)
    sem_t empty; // Semáforo para indicar a quantidade de espacos vazio no deposito
    sem_t full;  // Semáforo para indicar a quantidade de canetas no deposito
} DepositoCaneta;

// Variáveis globais
DepositoMaterial depositoMaterial;
DepositoCaneta depositoCaneta;

sem_t canetasFabrica; // Semáforo para indicar a quantidade de canetas disponiveis para enviar

// Funções correspondentes as diferentes threads
void *deposito_material(){

    int qntEnviada = 5;
    int tempoEnvio = 3;
    int stop = 0;

    while (TRUE - stop) {
        //Verifica se o deposito de canetas esta cheio
        sem_wait(&depositoCaneta.empty);

        pthread_mutex_lock(&depositoMaterial.mutex);

        //Envia a quantidade disponivel de material
        if(depositoMaterial.materialDeposito == 0){
            qntEnviada = 0;
            stop = 1;
            printf("Depósito de Material: Acabou a matéria-prima.\n");
        }
        else{
            if(depositoMaterial.materialDeposito < qntEnviada){
                qntEnviada = depositoMaterial.materialDeposito;
            }
            depositoMaterial.materialDeposito -= qntEnviada;
            depositoMaterial.materialFabrica += qntEnviada;
            printf("Depósito de Material: Enviando %d unidades de matéria-prima.\n", qntEnviada);
        }

        //Adiciona ao semaforo a quantidade enviada de material
        for(int i = 0; i < qntEnviada; i++){
            sem_post(&depositoMaterial.full);
        }

        pthread_mutex_unlock(&depositoMaterial.mutex);

        sem_post(&depositoCaneta.empty);

        sleep(tempoEnvio);
    }

    return NULL;
}

void *fabrica_caneta(){

    int tempoFabricacao = 1;

    while (TRUE) {
        //Verifica se o deposito de canetas esta cheio
        sem_wait(&depositoCaneta.empty);

        //Verifica se tem materia prima para fabricar
        sem_wait(&depositoMaterial.full);
        pthread_mutex_lock(&depositoMaterial.mutex);

        pthread_mutex_lock(&depositoCaneta.mutex);

        //Fabrica uma caneta
        depositoMaterial.materialFabrica--;
        depositoCaneta.canetasFabrica++;
        printf("Célula de fabricação de canetas: fabricou 1 caneta. Estoque de Material: %d\n", depositoMaterial.materialFabrica);

        //Adiciona no semaforo a quantidade de canetas que podem ser enviadas
        sem_post(&canetasFabrica);

        pthread_mutex_unlock(&depositoCaneta.mutex);
        pthread_mutex_unlock(&depositoMaterial.mutex);

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

    while (TRUE) {
        //Espera ter canetas disponiveis para serem enviadas
        sem_wait(&canetasFabrica);

        //Checa se o deposito de caneta esta cheio
        sem_wait(&depositoCaneta.empty);
        pthread_mutex_lock(&depositoCaneta.mutex);

        //Transfere uma caneta para o deposito
        depositoCaneta.canetasFabrica--;
        depositoCaneta.canetasDeposito++;
        printf("Depósito de Canetas: Enviada 1 caneta. Estoque de canetas: %d\n", depositoCaneta.canetasDeposito);

        //Adiciona uma caneta ao semaforo que mostra a quantidade de canetas do deposito
        sem_post(&depositoCaneta.full);

        pthread_mutex_unlock(&depositoCaneta.mutex);

        sleep(tempoEnvio);
    }

    return NULL;
}

void *comprador(){

    int tempoEspera = 5;
    int qntComprada;

    while (TRUE) {
        //Verifica se ha canetas disponiveis para a compra
        sem_wait(&depositoCaneta.full);
        pthread_mutex_lock(&depositoCaneta.mutex);

        qntComprada = 3;

        //Se nao tem a quantidade suficiente compra o que tem no estoque
        if(qntComprada > depositoCaneta.canetasDeposito){
            qntComprada = depositoCaneta.canetasDeposito;
        }

        //Compra as canetas
        depositoCaneta.canetasDeposito -= qntComprada;
        printf("Comprador: Comprou %d canetas.\n", qntComprada);

        //Decresce o semaforo de acordo com a qnt comprada
        for(int i = 1; i < qntComprada; i++){
            sem_wait(&depositoCaneta.full);
        }

        //Aumenta os espacos livres do deposito
        for(int i = 0; i < qntComprada; i++){
            sem_post(&depositoCaneta.empty);
        }

        pthread_mutex_unlock(&depositoCaneta.mutex);

        sleep(tempoEspera);
    }

    return NULL;
}

void *encerrar(){

    while(TRUE){
        pthread_mutex_lock(&depositoMaterial.mutex);
        pthread_mutex_lock(&depositoCaneta.mutex);

        //Quando as condicoes especificadas sao satisfeitas encerra o programa
        if(depositoMaterial.materialDeposito == 0 && depositoCaneta.canetasFabrica == 0 &&
         depositoCaneta.canetasDeposito == 0 && depositoMaterial.materialFabrica == 0){
            
            pthread_mutex_destroy(&depositoCaneta.mutex);
            sem_destroy(&depositoCaneta.empty);
            sem_destroy(&depositoCaneta.full);

            pthread_mutex_destroy(&depositoMaterial.mutex);
            sem_destroy(&depositoMaterial.full);

            sem_destroy(&canetasFabrica);

            exit(0);
        }

        pthread_mutex_unlock(&depositoMaterial.mutex);
        pthread_mutex_unlock(&depositoCaneta.mutex);
    }

    return NULL;
}

int main(int argc, char *argv[]){
    // Inicializando as variáveis e semáforos
    depositoMaterial.materialDeposito = 28;
    depositoMaterial.materialFabrica = 0;
    pthread_mutex_init(&depositoMaterial.mutex, NULL);
    sem_init(&depositoMaterial.full, 0, 0);

    depositoCaneta.canetasFabrica = 0;
    depositoCaneta.canetasDeposito = 0;
    pthread_mutex_init(&depositoCaneta.mutex, NULL);
    sem_init(&depositoCaneta.empty, 0, MAX_CANETAS);
    sem_init(&depositoCaneta.full, 0, 0);

    sem_init(&canetasFabrica, 0, 0);

    // Inicialização das threads
    pthread_t threads[5];
    pthread_create(&threads[0], NULL, deposito_material, NULL);
    pthread_create(&threads[1], NULL, fabrica_caneta, NULL);
    pthread_create(&threads[2], NULL, deposito_caneta, NULL);
    pthread_create(&threads[3], NULL, comprador, NULL);
    pthread_create(&threads[4], NULL, encerrar, NULL);

    // Executa as threads
    for (int i = 0; i < 5; ++i) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}