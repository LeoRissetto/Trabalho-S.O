/*
Leonardo Gueno Rissetto 13676482
Lucas Lima Romero 13676325
Luciano Gonçalves Lopes Filho 13676520
Marco Antonio Gaspar Garcia 11833581
Thiago Kashivagi Gonçalves 13676579

Para compilar:  gcc -o executavel main.c -lpthread
Para rodar:     ./executavel 1 2 3 4 5 6 7
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define TRUE 1

// Declaracao das variaveis globais e mecanismo de sincronizacao

// Deposito de Materia-prima
int materialDeposito;                                              // Quantidade de naterial no deposito
pthread_mutex_t mutexMaterialDeposito = PTHREAD_MUTEX_INITIALIZER; // Mutex para alterar as variaveis (zona critica)

// Fabrica de Canetas
int materialFabrica = 0;                                          // Quantidade de material na fabrica
int canetasFabrica = 0;                                           // Quantidade de canetas na fabrica
pthread_mutex_t mutexMaterialFabrica = PTHREAD_MUTEX_INITIALIZER; // Mutex para alterar a variavel material
pthread_mutex_t mutexCanetasFabrica = PTHREAD_MUTEX_INITIALIZER;  // Mutex para alterar a variavel Caneta
sem_t semMaterialFabrica;                                         // Semaforo para indicar a quantidade de material na fabrica
sem_t semCanetasFabrica;                                          // Semáforo para indicar a quantidade de canetas disponiveis para enviar

// Controle

// Deposito de Canetas
int max_canetas;                                                  // Quantidade maxima de canetas que podem ser armazenadas no deposito
int canetasDeposito = 0;                                          // Quantidade de canetas no deposito
pthread_mutex_t mutexCanetasDeposito = PTHREAD_MUTEX_INITIALIZER; // Mutex para alterar as variaveis (zona critica)
sem_t semEspacosVaziosDeposito;                                   // Semáforo para indicar a quantidade de espacos vazio no deposito
sem_t semCanetasDeposito;                                         // Semáforo para indicar a quantidade de canetas no deposito

// Funções correspondentes as diferentes threads
void *deposito_material(void *arg)
{
    char **argv = (char **)arg;

    int qntEnviada = atoi(argv[2]);
    int tempoEnvio = atoi(argv[3]);

    int stop = 0;

    while (TRUE - stop)
    {
        // Controle

        pthread_mutex_lock(&mutexMaterialDeposito);
        pthread_mutex_lock(&mutexMaterialFabrica);

        // Envia a quantidade disponivel de material
        if (materialDeposito == 0)
        {
            qntEnviada = 0;
            stop = 1;
            printf("Depósito de Material: Acabou a matéria-prima.\n");
        }
        else
        {   
            // Se houver menos matéria prima do que a qtdEnviada normalmente:
            if (materialDeposito < qntEnviada)
            {
                qntEnviada = materialDeposito;
            }
            materialDeposito -= qntEnviada;
            materialFabrica += qntEnviada;
            printf("Depósito de Material: Enviando %d unidades de matéria-prima.\n", qntEnviada);
        }

        pthread_mutex_unlock(&mutexMaterialFabrica);
        pthread_mutex_unlock(&mutexMaterialDeposito);

        // Adiciona ao semaforo a quantidade enviada de material
        for (int i = 0; i < qntEnviada; i++)
        {
            sem_post(&semMaterialFabrica);
        }

        sleep(tempoEnvio);
    }

    return NULL;
}

void *fabrica_caneta(void *arg)
{
    char **argv = (char **)arg;

    int tempoFabricacao = atoi(argv[4]);

    while (TRUE)
    {
        // Verifica se tem espaco no deposito de canetas
        sem_wait(&semEspacosVaziosDeposito);

        // Verifica se tem materia prima para fabricar
        sem_wait(&semMaterialFabrica);

        pthread_mutex_lock(&mutexMaterialFabrica);
        pthread_mutex_lock(&mutexCanetasFabrica);

        // Fabrica a/as canetas
        materialFabrica--;
        canetasFabrica++;
        printf("Célula de fabricação de canetas: fabricou 1 caneta\n");

        pthread_mutex_unlock(&mutexCanetasFabrica);
        pthread_mutex_unlock(&mutexMaterialFabrica);

        // Adiciona no semaforo a quantidade de canetas que podem ser enviadas
        sem_post(&semCanetasFabrica);

        sleep(tempoFabricacao);
    }

    return NULL;
}

void *controle()
{
    while (TRUE)
    {
        pthread_mutex_lock(&mutexMaterialDeposito);

        pthread_mutex_lock(&mutexMaterialFabrica);
        pthread_mutex_lock(&mutexCanetasFabrica);

        pthread_mutex_lock(&mutexCanetasDeposito);

        if(materialDeposito + materialFabrica +
        canetasFabrica + canetasDeposito == 0) {
            exit(0);
        }

        pthread_mutex_unlock(&mutexCanetasDeposito);

        pthread_mutex_unlock(&mutexCanetasFabrica);
        pthread_mutex_unlock(&mutexMaterialFabrica);

        pthread_mutex_unlock(&mutexMaterialDeposito);
    }

    return NULL;
}

void *deposito_caneta()
{
    while (TRUE)
    {
        // Espera ter canetas disponiveis para serem enviadas
        sem_wait(&semCanetasFabrica);

        pthread_mutex_lock(&mutexCanetasFabrica);
        pthread_mutex_lock(&mutexCanetasDeposito);

        // Transfere uma caneta para o deposito
        canetasFabrica--;
        canetasDeposito++;
        printf("Depósito de Canetas: Enviada 1 caneta. Estoque de canetas: %d\n", canetasDeposito);

        pthread_mutex_unlock(&mutexCanetasDeposito);
        pthread_mutex_unlock(&mutexCanetasFabrica);

        // Adiciona uma caneta ao semaforo que mostra a quantidade de canetas do deposito
        sem_post(&semCanetasDeposito);

        sleep(1);
    }

    return NULL;
}

void *comprador(void *arg)
{
    char **argv = (char **)arg;

    int tempoEspera = atoi(argv[7]);
    int qntComprada;

    while (TRUE)
    {
        // Verifica se ha canetas disponiveis para a compra
        sem_wait(&semCanetasDeposito);

        pthread_mutex_lock(&mutexCanetasDeposito);

        qntComprada = atoi(argv[6]);

        // Se nao tem a quantidade suficiente compra o que tem no estoque
        if (qntComprada > canetasDeposito)
        {
            qntComprada = canetasDeposito;
        }

        // Decresce o semaforo de acordo com a qnt comprada
        for (int i = 1; i < qntComprada; i++)
        {
            sem_wait(&semCanetasDeposito);
        }

        // Compra as canetas
        canetasDeposito -= qntComprada;
        printf("Comprador: Comprou %d canetas.\n", qntComprada);

        pthread_mutex_unlock(&mutexCanetasDeposito);

        // Aumenta os espacos livres do deposito
        for (int i = 0; i < qntComprada; i++)
        {
            sem_post(&semEspacosVaziosDeposito);
        }

        sleep(tempoEspera);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    // Lendo as variaveis do usuario
    if (argc != 8)
    {
        printf("Uso: %s <quantidade_inicial_deposito_material> <quantidade_enviada_material> <tempo_envio_material> <tempo_fabricacao> <quantidade_maxima_canetas> <quantidade_comprada> <tempo_espera_compra>\n", argv[0]);
        return 1;
    }

    // Inicializando as variáveis, travas e semáforos

    // variaveis deposito material
    materialDeposito = atoi(argv[1]);

    // variaveis fabrica
    sem_init(&semMaterialFabrica, 0, 0);
    sem_init(&semCanetasFabrica, 0, 0);

    // variaveis controle

    // variaveis deposito caneta
    max_canetas = atoi(argv[5]);
    sem_init(&semEspacosVaziosDeposito, 0, atoi(argv[5]));
    sem_init(&semCanetasDeposito, 0, 0);

    // Inicialização das threads
    pthread_t threads[4];
    pthread_create(&threads[0], NULL, deposito_material, (void *)argv);
    pthread_create(&threads[1], NULL, fabrica_caneta, (void *)argv);
    pthread_create(&threads[2], NULL, controle, NULL);
    pthread_create(&threads[3], NULL, deposito_caneta, NULL);
    pthread_create(&threads[4], NULL, comprador, (void *)argv);

    // Executa as threads
    for (int i = 0; i < 4; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}