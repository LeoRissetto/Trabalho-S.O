#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_MATERIAL 100
#define MAX_CANETAS 50
#define TRUE 1

// Definição das estruturas de dados
typedef struct {
    int material;
    int materialEnviado;
} DepositoMaterial;

typedef struct {
    int canetas;
    int canetasEnviadas;
} DepositoCaneta;

// Variáveis globais
DepositoMaterial depositoMaterial = {100, 0};
DepositoCaneta depositoCaneta = {0, 0};

sem_t canetas_disponiveis;
pthread_mutex_t deposito_material_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t deposito_caneta_mutex = PTHREAD_MUTEX_INITIALIZER;

// Funções auxiliares
void *deposito_material(void *arg){

    char **argv = (char **)arg;
    int material_enviado = atoi(argv[2]);
    int tempo_envio_material = atoi(argv[3]);

    while (TRUE){
        pthread_mutex_lock(&deposito_material_mutex);
        if (depositoMaterial.material >= material_enviado){
            depositoMaterial.material -= material_enviado;
            depositoMaterial.materialEnviado += material_enviado;
            printf("Depósito de matéria prima: enviado %d unidades, total: %d\n", material_enviado, depositoMaterial.material);
        }
        pthread_mutex_unlock(&deposito_material_mutex);
        sleep(tempo_envio_material);
    }

    return NULL;
}

void *fabrica_caneta(void *arg){

    char **argv = (char **)arg;
    int tempo_fabricacao = atoi(argv[4]);

    while (TRUE){
        pthread_mutex_lock(&deposito_material_mutex);
        pthread_mutex_lock(&deposito_caneta_mutex);
        if (depositoMaterial.materialEnviado >= 1 && depositoCaneta.canetas < MAX_CANETAS){
            depositoMaterial.materialEnviado--;
            depositoCaneta.canetas++;
            printf("Célula de fabricação de canetas: fabricou 1 caneta, total: %d\n", depositoCaneta.canetas);
            sem_post(&canetas_disponiveis); // Sinaliza que há canetas disponíveis
        }
        pthread_mutex_unlock(&deposito_caneta_mutex);
        pthread_mutex_unlock(&deposito_material_mutex);
        sleep(tempo_fabricacao);
    }

    return NULL;
}

void *controle(void *arg){

    char **argv = (char **)arg;
    int canetas_compradas = atoi(argv[6]);

    while (TRUE) {
        pthread_mutex_lock(&deposito_caneta_mutex);
        if (depositoCaneta.canetas >= canetas_compradas) {
            printf("Depósito de canetas: capacidade suficiente para vender\n");
        }
        pthread_mutex_unlock(&deposito_caneta_mutex);
        sleep(1);
    }

    return NULL;
}

void *deposito_caneta(void *arg){

    char **argv = (char **)arg;
    int canetas_enviadas = atoi(argv[5]);
    int tempo_envio_caneta = atoi(argv[6]);

    while (TRUE){
        pthread_mutex_lock(&deposito_caneta_mutex);
        if (depositoCaneta.canetas < MAX_CANETAS && depositoCaneta.canetas >= canetas_enviadas){
            depositoCaneta.canetas -= canetas_enviadas;
            depositoCaneta.canetasEnviadas += canetas_enviadas;
            printf("Depósito de canetas: adicionado %d canetas, total: %d\n", canetas_enviadas, depositoCaneta.canetas);
        }
        pthread_mutex_unlock(&deposito_caneta_mutex);
        sleep(tempo_envio_caneta);
    }

    return NULL;
}

void *comprador(void *arg){

    char **argv = (char **)arg;
    int canetas_compradas = atoi(argv[6]);
    int tempo_espera_compra = atoi(argv[7]);

    while (TRUE){
        sem_wait(&canetas_disponiveis); // Espera por canetas disponíveis
        pthread_mutex_lock(&deposito_caneta_mutex);
        if (depositoCaneta.canetasEnviadas >= canetas_compradas){
            printf("Comprador: comprando %d canetas...\n", canetas_compradas);
            depositoCaneta.canetasEnviadas -= canetas_compradas;
            printf("Comprador: canetas compradas, total: %d\n", depositoCaneta.canetasEnviadas);
        }
        pthread_mutex_unlock(&deposito_caneta_mutex);
        sleep(tempo_espera_compra);
    }

    return NULL;
}

int main(int argc, char *argv[]){

    // Verificação dos argumentos de entrada
    if (argc != 8){
        fprintf(stderr, "Uso: %s <qtde_material> <qtde_enviada> <tempo_envio> <tempo_fabricar> <qtde_max_pens> <qtde_compradas> <tempo_compra>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Inicialização do semáforo
    sem_init(&canetas_disponiveis, 0, 0);

    // Inicialização das threads
    pthread_t threads[5];
    pthread_create(&threads[0], NULL, deposito_material, (void *)argv);
    pthread_create(&threads[1], NULL, fabrica_caneta, (void *)argv);
    pthread_create(&threads[2], NULL, controle, (void *)argv);
    pthread_create(&threads[3], NULL, deposito_caneta, (void *)argv);
    pthread_create(&threads[4], NULL, comprador, (void *)argv);

    // Aguardar o término das threads
    for (int i = 0; i < 5; ++i){
        pthread_join(threads[i], NULL);
    }

    // Destruir o semáforo
    sem_destroy(&canetas_disponiveis);

    return 0;
}
