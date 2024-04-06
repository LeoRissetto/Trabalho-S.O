#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_MATERIAL 100
#define MAX_PENS 50

// Definição das estruturas de dados
typedef struct {
    int material;
    int pens;
} Warehouse;

// Variáveis globais
Warehouse warehouse = {0, 0};
sem_t material_sem, pens_sem;
pthread_mutex_t warehouse_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t warehouse_cond = PTHREAD_COND_INITIALIZER;

// Funções auxiliares
void *material_depositor(void *arg) {
    char **argv = (char **)arg;
    int material_sent = atoi(argv[2]);
    int time_between_sends = atoi(argv[3]);

    while (1) {
        sem_wait(&material_sem);
        if (warehouse.material < MAX_MATERIAL) {
            warehouse.material += material_sent;
            printf("Depósito de matéria prima: adicionado %d unidades, total: %d\n", material_sent, warehouse.material);
        }
        sem_post(&material_sem);
        sleep(time_between_sends);
    }
    return NULL;
}

void *pen_manufacturer(void *arg) {
    char **argv = (char **)arg;
    int time_to_manufacture_pen = atoi(argv[4]);

    while (1) {
        sem_wait(&material_sem);
        sem_wait(&pens_sem);
        if (warehouse.material >= 1 && warehouse.pens < MAX_PENS) {
            warehouse.material--;
            warehouse.pens++;
            printf("Célula de fabricação de canetas: fabricou 1 caneta, total: %d\n", warehouse.pens);
        }
        sem_post(&pens_sem);
        sem_post(&material_sem);
        sleep(time_to_manufacture_pen);
    }
    return NULL;
}

void *control(void *arg) {
    while (1) {
        pthread_mutex_lock(&warehouse_mutex);
        if (warehouse.pens >= 1) {
            printf("Depósito de canetas: capacidade suficiente para vender\n");
            pthread_cond_signal(&warehouse_cond);
        }
        pthread_mutex_unlock(&warehouse_mutex);
        sleep(1);
    }
    return NULL;
}

void *buyer(void *arg) {
    char **argv = (char **)arg;
    int pens_bought = atoi(argv[6]);
    int time_between_buys = atoi(argv[7]);

    while (1) {
        pthread_mutex_lock(&warehouse_mutex);
        while (warehouse.pens < pens_bought) {
            printf("Comprador: aguardando canetas disponíveis...\n");
            pthread_cond_wait(&warehouse_cond, &warehouse_mutex);
        }
        printf("Comprador: comprando %d canetas...\n", pens_bought);
        warehouse.pens -= pens_bought;
        printf("Comprador: canetas compradas, total: %d\n", warehouse.pens);
        pthread_mutex_unlock(&warehouse_mutex);
        sleep(time_between_buys);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    // Verificação dos argumentos de entrada
    if (argc != 8) {
        fprintf(stderr, "Uso: %s <qtde_material> <qtde_enviada> <tempo_envio> <tempo_fabricar> <qtde_max_pens> <qtde_compradas> <tempo_compra>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Inicialização dos semáforos
    sem_init(&material_sem, 0, 1);
    sem_init(&pens_sem, 0, 1);

    // Inicialização das threads
    pthread_t threads[5];
    pthread_create(&threads[0], NULL, material_depositor, (void *)argv);
    pthread_create(&threads[1], NULL, pen_manufacturer, (void *)argv);
    pthread_create(&threads[2], NULL, control, NULL);
    pthread_create(&threads[3], NULL, buyer, (void *)argv);

    // Aguardar o término das threads
    for (int i = 0; i < 4; ++i) {
        pthread_join(threads[i], NULL);
    }

    // Destruir os semáforos
    sem_destroy(&material_sem);
    sem_destroy(&pens_sem);

    return 0;
}