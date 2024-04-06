#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

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
DepositoMaterial depositoMaterial = {30, 0};
DepositoCaneta depositoCaneta = {0, 0};

sem_t canetas_disponiveis;
pthread_mutex_t deposito_material_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t deposito_caneta_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t deposito_caneta_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t deposito_material_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t fabrica_cond = PTHREAD_COND_INITIALIZER;

// Funções auxiliares
void *deposito_material(void *arg){

    char **argv = (char **)arg;
    int material_enviado = atoi(argv[1]);
    int tempo_envio_material = atoi(argv[2]);

    while (TRUE){
        pthread_mutex_lock(&deposito_material_mutex);
        while (depositoCaneta.canetasEnviadas >= MAX_CANETAS){
            printf("Deposito Material: deposito de canetas lotado...\n");
            pthread_cond_wait(&deposito_material_cond, &deposito_material_mutex);
        }
        if (depositoMaterial.material >= material_enviado){
            depositoMaterial.material -= material_enviado;
            depositoMaterial.materialEnviado += material_enviado;
            printf("Depósito de matéria prima: enviado %d unidades\n", material_enviado);
        }
        pthread_mutex_unlock(&deposito_material_mutex);
        sleep(tempo_envio_material);
    }

    return NULL;
}

void *fabrica_caneta(void *arg){

    char **argv = (char **)arg;
    int tempo_fabricacao = atoi(argv[3]);

    while (TRUE){
        pthread_mutex_lock(&deposito_material_mutex);
        pthread_mutex_lock(&deposito_caneta_mutex);
        while (depositoCaneta.canetasEnviadas >= MAX_CANETAS || depositoMaterial.materialEnviado == 0){
            if(depositoMaterial.materialEnviado == 0){
                printf("Deposito Material: acabou a materia prima...\n");
            }
            else{
                printf("Deposito Material: deposito de canetas lotado...\n");
            }
            pthread_cond_wait(&fabrica_cond, &deposito_caneta_mutex);
        }
        depositoMaterial.materialEnviado--;
        depositoCaneta.canetas++;
        printf("Célula de fabricação de canetas: fabricou 1 caneta\n");
        sem_post(&canetas_disponiveis); // Sinaliza que há canetas disponíveis
        pthread_mutex_unlock(&deposito_caneta_mutex);
        pthread_mutex_unlock(&deposito_material_mutex);
        sleep(tempo_fabricacao);
    }

    return NULL;
}

void *controle(void *arg){

    char **argv = (char **)arg;

    while(TRUE){
        pthread_mutex_lock(&deposito_material_mutex);
        pthread_mutex_lock(&deposito_caneta_mutex);
        if (depositoCaneta.canetasEnviadas < MAX_CANETAS){
            pthread_cond_signal(&deposito_material_cond);
            if(depositoMaterial.materialEnviado >= 1){
                pthread_cond_signal(&fabrica_cond);
            }
        pthread_mutex_unlock(&deposito_material_mutex);
        pthread_mutex_unlock(&deposito_caneta_mutex);
        }
        pthread_mutex_unlock(&deposito_material_mutex);
        pthread_mutex_unlock(&deposito_caneta_mutex);
    }

    return NULL;
}

void *deposito_caneta(void *arg){

    char **argv = (char **)arg;
    int canetas_compradas = atoi(argv[4]);
    int canetas_enviadas = atoi(argv[6]);
    int tempo_envio_caneta = atoi(argv[5]);

    while (TRUE){
        pthread_mutex_lock(&deposito_caneta_mutex);
        if (depositoCaneta.canetas >= canetas_enviadas){
            depositoCaneta.canetas -= canetas_enviadas;
            depositoCaneta.canetasEnviadas += canetas_enviadas;
            printf("Depósito de canetas: adicionado %d canetas\n", canetas_enviadas);
            if (depositoCaneta.canetasEnviadas >= canetas_compradas){
                printf("Depósito de canetas: capacidade suficiente para vender\n");
                pthread_cond_signal(&deposito_caneta_cond);
            }
        }
        pthread_mutex_unlock(&deposito_caneta_mutex);
        sleep(tempo_envio_caneta);
    }

    return NULL;
}

void *comprador(void *arg){

    char **argv = (char **)arg;
    int canetas_compradas = atoi(argv[4]);
    int tempo_espera_compra = atoi(argv[7]);

    while (TRUE){
        sem_wait(&canetas_disponiveis); // Espera por canetas disponíveis
        pthread_mutex_lock(&deposito_caneta_mutex);
        while (depositoCaneta.canetasEnviadas < canetas_compradas){
            printf("Comprador: aguardando canetas disponíveis...\n");
            pthread_cond_wait(&deposito_caneta_cond, &deposito_caneta_mutex);
        }
        printf("Comprador: comprando %d canetas...\n", canetas_compradas);
        depositoCaneta.canetasEnviadas -= canetas_compradas;
        pthread_mutex_unlock(&deposito_caneta_mutex);
        sleep(tempo_espera_compra);
    }

    return NULL;
}

void *encerrar(void *arg){

    while(TRUE){
        pthread_mutex_lock(&deposito_material_mutex);
        pthread_mutex_lock(&deposito_caneta_mutex);
        if(depositoMaterial.materialEnviado == 0 && depositoCaneta.canetasEnviadas == 0 && depositoCaneta.canetas == 0){
            exit(0);
        }
        pthread_mutex_unlock(&deposito_caneta_mutex);
        pthread_mutex_unlock(&deposito_material_mutex);
    }

    return NULL;
}

int main(int argc, char *argv[]){

    // Verificação dos argumentos de entrada
    if (argc != 8){
        fprintf(stderr, "Uso: %s <qtde_enviada> <tempo_envio_material> <tempo_fabricar> <qtde_compradas> <tempo_envio_caneta> <max_canetas_enviadas> <tempo_compra>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Inicialização do semáforo
    sem_init(&canetas_disponiveis, 0, 0);

    // Inicialização das threads
    pthread_t threads[6];
    pthread_create(&threads[0], NULL, deposito_material, (void *)argv);
    pthread_create(&threads[1], NULL, fabrica_caneta, (void *)argv);
    pthread_create(&threads[2], NULL, controle, (void *)argv);
    pthread_create(&threads[3], NULL, deposito_caneta, (void *)argv);
    pthread_create(&threads[4], NULL, comprador, (void *)argv);
    pthread_create(&threads[5], NULL, encerrar, (void *)argv);

    // Aguardar o término das threads
    for (int i = 0; i < 6; ++i){
        pthread_join(threads[i], NULL);
    }

    // Destruir o semáforo
    sem_destroy(&canetas_disponiveis);


    return 0;
}
