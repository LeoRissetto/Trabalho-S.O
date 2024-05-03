/*

Leonardo Gueno Rissetto 13676482
Lucas Lima Romero 13676325
Luciano Gonçalves Lopes Filho 13676520
Marco Antonio Gaspar Garcia 11833581
Thiago Kashivagui Gonçalves 13676579

Para compilar:  gcc main.c -o programa -lpthread
Para rodar:     ./programa X X X X X X X

*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define TRUE 1

// Declaração das variáveis globais e mecanismos de sincronização

// Depósito de Matéria-Prima
int encerrarDepositoMaterial = 0;
int materialDeposito;                                              // Quantidade de material no depósito
pthread_mutex_t mutexMaterialDeposito = PTHREAD_MUTEX_INITIALIZER; // Mutex para alterar as variáveis (zona crítica)

// Fábrica de Canetas
int materialFabrica = 0;                                          // Quantidade de material na fábrica
int canetasFabrica = 0;                                           // Quantidade de canetas na fábrica
pthread_mutex_t mutexMaterialFabrica = PTHREAD_MUTEX_INITIALIZER; // Mutex para alterar a variavel material
pthread_mutex_t mutexCanetasFabrica = PTHREAD_MUTEX_INITIALIZER;  // Mutex para alterar a variavel Caneta
sem_t semMaterialFabrica;                                         // Semaforo para indicar a quantidade de material na fabrica
sem_t semCanetasFabrica;                                          // Semáforo para indicar a quantidade de canetas disponiveis para enviar

// Controle
pthread_mutex_t mutexControle = PTHREAD_MUTEX_INITIALIZER; // Mutex para fazer o Controle
pthread_cond_t condControle = PTHREAD_COND_INITIALIZER;    // Condicional para fzer o Controle

// Depósito de Canetas
int max_canetas;                                                  // Quantidade máxima de canetas que podem ser armazenadas no depósito
int canetasDeposito = 0;                                          // Quantidade de canetas no depósito
pthread_mutex_t mutexCanetasDeposito = PTHREAD_MUTEX_INITIALIZER; // Mutex para zzz (zona crítica)
sem_t semEspacosVaziosDeposito;                                   // Semáforo para indicar a quantidade de espaços vazios no depósito
sem_t semCanetasDeposito;                                         // Semáforo para indicar a quantidade de canetas no depósito

// Comprador
int totalVendas;
pthread_mutex_t mutexTotalVendas = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condTotalVendas = PTHREAD_COND_INITIALIZER;

// Funções correspondentes as diferentes threads
void *deposito_material(void *arg)
{
    char **argv = (char **)arg;

    int qntEnviada = atoi(argv[2]);
    int tempoEnvio = atoi(argv[3]);

    int stop = 0;

    while (!encerrarDepositoMaterial)
    {
        // Espera um sinal do controle para prosseguir
        pthread_mutex_lock(&mutexControle);
        pthread_cond_wait(&condControle, &mutexControle);
        pthread_mutex_unlock(&mutexControle);

        pthread_mutex_lock(&mutexMaterialDeposito);
        pthread_mutex_lock(&mutexMaterialFabrica);

        // Envia a quantidade disponível de material
        if (materialDeposito == 0)
        {
            qntEnviada = 0;
            encerrarDepositoMaterial = 1;
            printf("Depósito de Material: Acabou a matéria-prima.\n");
        }
        else
        {
            if (materialDeposito < qntEnviada)
            {
                qntEnviada = materialDeposito;
            }
            materialDeposito -= qntEnviada;
            materialFabrica += qntEnviada;
            printf("Depósito de Material: Enviando %d unidades de matéria-prima.\n", qntEnviada);
        }

        // Adiciona ao semáforo a quantidade enviada de material
        for (int i = 0; i < qntEnviada; i++)
        {
            sem_post(&semMaterialFabrica);
        }

        pthread_mutex_unlock(&mutexMaterialFabrica);
        pthread_mutex_unlock(&mutexMaterialDeposito);

        sleep(tempoEnvio);
    }

    return NULL;
}

void *fabrica_caneta(void *arg)
{
    char **argv = (char **)arg;

    int tempoFabricacao = atoi(argv[4]);

    int qntFabricada;

    while (TRUE)
    {
        pthread_mutex_lock(&mutexTotalVendas);
        if (totalVendas < atoi(argv[1]))
        {
            pthread_cond_wait(&condTotalVendas, &mutexTotalVendas);
        }
        else
        {
            pthread_mutex_unlock(&mutexTotalVendas);
            break;
        }
        pthread_mutex_unlock(&mutexTotalVendas);

        // Espera um sinal do controle para prosseguir
        pthread_mutex_lock(&mutexControle);
        pthread_cond_wait(&condControle, &mutexControle);
        pthread_mutex_unlock(&mutexControle);

        // Verifica se tem matéria-prima para fabricar
        sem_wait(&semMaterialFabrica);

        pthread_mutex_lock(&mutexMaterialFabrica);
        pthread_mutex_lock(&mutexCanetasFabrica);

        // Calcula a quantidaded de canetas que deve ser fabricada
        sem_getvalue(&semEspacosVaziosDeposito, &qntFabricada);
        qntFabricada -= canetasFabrica;

        if (qntFabricada > materialFabrica)
        {
            qntFabricada = materialFabrica;
        }

        // Decresce o número de materiais da fábrica de acordo com a quantidade produzida
        for (int i = 1; i < qntFabricada; i++)
        {
            sem_wait(&semMaterialFabrica);
        }

        // Fabrica as canetas
        materialFabrica -= qntFabricada;
        canetasFabrica += qntFabricada;
        printf("Célula de fabricação de canetas: fabricou %d canetas\n", qntFabricada);

        // Adiciona ao semáforo a quantidade de canetas que podem ser enviadas
        for (int i = 0; i < qntFabricada; i++)
        {
            sem_post(&semCanetasFabrica);
        }

        pthread_mutex_unlock(&mutexCanetasFabrica);
        pthread_mutex_unlock(&mutexMaterialFabrica);

        sleep(tempoFabricacao * qntFabricada);
    }

    return NULL;
}

void *controle(void *arg)
{
    char **argv = (char **)arg;

    while (TRUE)
    {
        pthread_mutex_lock(&mutexTotalVendas);
        if (totalVendas < atoi(argv[1]))
        {
            pthread_cond_wait(&condTotalVendas, &mutexTotalVendas);
        }
        else
        {
            pthread_mutex_unlock(&mutexTotalVendas);
            break;
        }
        pthread_mutex_unlock(&mutexTotalVendas);

        pthread_mutex_lock(&mutexControle);

        pthread_mutex_lock(&mutexCanetasFabrica);
        pthread_mutex_lock(&mutexCanetasDeposito);

        // Sinaliza que a fábrica pode produzir e que o depósito de matéria-prima pode enviar
        if (canetasDeposito + canetasFabrica < max_canetas)
        {
            pthread_cond_broadcast(&condControle);
        }

        pthread_mutex_unlock(&mutexCanetasDeposito);
        pthread_mutex_unlock(&mutexCanetasFabrica);

        pthread_mutex_unlock(&mutexControle);
    }

    return NULL;
}

void *deposito_caneta(void *arg)
{
    char **argv = (char **)arg;

    while (TRUE)
    {
        pthread_mutex_lock(&mutexTotalVendas);
        if (totalVendas < atoi(argv[1]))
        {
            pthread_cond_wait(&condTotalVendas, &mutexTotalVendas);
        }
        else
        {
            pthread_mutex_unlock(&mutexTotalVendas);
            break;
        }
        pthread_mutex_unlock(&mutexTotalVendas);

        // Espera ter canetas disponíveis para serem enviadas
        sem_wait(&semCanetasFabrica);

        // Decresce os espacos vazios do deposito
        sem_wait(&semEspacosVaziosDeposito);

        pthread_mutex_lock(&mutexCanetasFabrica);
        pthread_mutex_lock(&mutexCanetasDeposito);

        // Transfere uma caneta para o depósito
        canetasFabrica--;
        canetasDeposito++;
        printf("Depósito de Canetas: Enviada 1 caneta. Estoque de canetas: %d\n", canetasDeposito);

        // Adiciona uma caneta ao semáforo que controla a quantidade de canetas do depósito
        sem_post(&semCanetasDeposito);

        pthread_mutex_unlock(&mutexCanetasDeposito);
        pthread_mutex_unlock(&mutexCanetasFabrica);

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
        pthread_mutex_lock(&mutexTotalVendas);
        if (totalVendas < atoi(argv[1]))
        {
            pthread_cond_wait(&condTotalVendas, &mutexTotalVendas);
        }
        else
        {
            pthread_mutex_unlock(&mutexTotalVendas);
            break;
        }
        pthread_mutex_unlock(&mutexTotalVendas);

        // Verifica se há canetas disponíveis para a compra
        sem_wait(&semCanetasDeposito);

        pthread_mutex_lock(&mutexCanetasDeposito);

        qntComprada = atoi(argv[6]);

        // Se não tem a quantidade suficiente, compra o que tem no estoque
        if (qntComprada > canetasDeposito)
        {
            qntComprada = canetasDeposito;
        }

        // Decresce o semáforo de acordo com a quantidade comprada
        for (int i = 1; i < qntComprada; i++)
        {
            sem_wait(&semCanetasDeposito);
        }

        // Compra as canetas
        canetasDeposito -= qntComprada;
        printf("Comprador: Comprou %d canetas.\n", qntComprada);

        pthread_mutex_lock(&mutexTotalVendas);
        totalVendas += qntComprada;
        pthread_mutex_unlock(&mutexTotalVendas);

        // Aumenta os espaços livres do depósito
        for (int i = 0; i < qntComprada; i++)
        {
            sem_post(&semEspacosVaziosDeposito);
        }

        pthread_mutex_unlock(&mutexCanetasDeposito);

        sleep(tempoEspera);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    // Lendo as variáveis do usuário
    if (argc != 8)
    {
        printf("Uso: %s <quantidade_inicial_deposito_material> <quantidade_enviada_material> <tempo_envio_material> <tempo_fabricacao> <quantidade_maxima_canetas> <quantidade_comprada> <tempo_espera_compra>\n", argv[0]);
        return 1;
    }

    /* Inicializando as variáveis, mutexes e semáforos */

    // Variáveis do Depósito de Material
    materialDeposito = atoi(argv[1]);

    // Variáveis da Fábrica de Canetas
    sem_init(&semMaterialFabrica, 0, 0);
    sem_init(&semCanetasFabrica, 0, 0);

    // Variáveis do Depósito de Canetas
    max_canetas = atoi(argv[5]);
    sem_init(&semEspacosVaziosDeposito, 0, atoi(argv[5]));
    sem_init(&semCanetasDeposito, 0, 0);

    // Inicialização das Threads
    pthread_t threads[5];
    pthread_create(&threads[0], NULL, deposito_material, (void *)argv);
    pthread_create(&threads[1], NULL, fabrica_caneta, (void *)argv);
    pthread_create(&threads[2], NULL, controle, (void *)argv);
    pthread_create(&threads[3], NULL, deposito_caneta, (void *)argv);
    pthread_create(&threads[4], NULL, comprador, (void *)argv);

    while (TRUE)
    {
        pthread_mutex_lock(&mutexTotalVendas);
        if (totalVendas < atoi(argv[1]))
        {
            pthread_cond_broadcast(&condTotalVendas);
        }
        pthread_mutex_unlock(&mutexTotalVendas);
    }

    // Aguarda o término das Threads
    for (int i = 0; i < 5; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // Liberando a memória

    pthread_mutex_destroy(&mutexMaterialDeposito);
    pthread_mutex_destroy(&mutexCanetasFabrica);
    pthread_mutex_destroy(&mutexMaterialFabrica);
    pthread_mutex_destroy(&mutexCanetasDeposito);
    pthread_mutex_destroy(&mutexControle);

    sem_destroy(&semMaterialFabrica);
    sem_destroy(&semCanetasFabrica);
    sem_destroy(&semEspacosVaziosDeposito);
    sem_destroy(&semCanetasDeposito);

    printf("FIM DO PROGRAMA\n");

    return 0;
}