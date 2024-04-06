#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_MATERIAL 100
#define MAX_PENS 50
#define TRUE 1

// Definição das estruturas de dados
typedef struct {
    int material;
} DepositoMaterial;

typedef struct {
    int canetas;
} DepositoCaneta;

// Variáveis globais
DepositoMaterial depositoMaterial = {0};
DepositoCaneta depositoCaneta = {0};

sem_t material_sem, caneta_sem;
pthread_mutex_t deposito_material_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t deposito_caneta_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t deposito_caneta_cond = PTHREAD_COND_INITIALIZER;

// Funções auxiliares
void *deposito_material(void *arg){

    char **argv = (char **)arg;
    int material_enviado = atoi(argv[2]);
    int tempo_envio_material = atoi(argv[3]);

    while (TRUE){
        pthread_mutex_lock(&deposito_material_mutex);
        if (depositoMaterial.material < MAX_MATERIAL){
            depositoMaterial.material += material_enviado;
            printf("Depósito de matéria prima: adicionado %d unidades, total: %d\n", material_enviado, depositoMaterial.material);
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
        if (depositoMaterial.material >= 1 && depositoCaneta.canetas < MAX_PENS){
            depositoMaterial.material--;
            depositoCaneta.canetas++;
            printf("Célula de fabricação de canetas: fabricou 1 caneta, total: %d\n", depositoCaneta.canetas);
        }
        pthread_mutex_unlock(&deposito_caneta_mutex);
        pthread_mutex_unlock(&deposito_material_mutex);
        sleep(tempo_fabricacao);
    }

    return NULL;
}

void *controle(void *arg){

    while (TRUE) {
        pthread_mutex_lock(&deposito_caneta_mutex);
        if (depositoCaneta.canetas >= 1) {
            printf("Depósito de canetas: capacidade suficiente para vender\n");
            pthread_cond_signal(&deposito_caneta_cond);
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
        if (depositoCaneta.canetas < MAX_PENS){
            depositoCaneta.canetas += canetas_enviadas;
            printf("Depósito de canetas: adicionado %d canetas, total: %d\n", canetas_enviadas, depositoCaneta.canetas);
            pthread_cond_signal(&deposito_caneta_cond); // Notifica os compradores de que há canetas disponíveis
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
        pthread_mutex_lock(&deposito_caneta_mutex);
        while (depositoCaneta.canetas < canetas_compradas){
            printf("Comprador: aguardando canetas disponíveis...\n");
            pthread_cond_wait(&deposito_caneta_cond, &deposito_caneta_mutex);
        }
        printf("Comprador: comprando %d canetas...\n", canetas_compradas);
        depositoCaneta.canetas -= canetas_compradas;
        printf("Comprador: canetas compradas, total: %d\n", depositoCaneta.canetas);
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

    // Inicialização dos semáforos
    sem_init(&material_sem, 0, 1);
    sem_init(&caneta_sem, 0, 1);

    // Inicialização das threads
    pthread_t threads[5];
    pthread_create(&threads[0], NULL, deposito_material, (void *)argv);
    pthread_create(&threads[1], NULL, fabrica_caneta, (void *)argv);
    pthread_create(&threads[2], NULL, controle, NULL);
    pthread_create(&threads[3], NULL, deposito_caneta, (void *)argv);
    pthread_create(&threads[4], NULL, comprador, (void *)argv);

    // Inicialização dos semáforos
    sem_init(&material_sem, 0, 1);
    sem_init(&caneta_sem, 0, 1);

    // Aguardar o término das threads
    for (int i = 0; i < 5; ++i){
        pthread_join(threads[i], NULL);
    }

    // Destruir os semáforos
    sem_destroy(&material_sem);
    sem_destroy(&caneta_sem);

    return 0;
}