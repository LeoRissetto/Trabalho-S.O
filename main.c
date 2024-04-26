#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define TRUE 1

// Definição das estruturas de dados
typedef struct {
    int material; // Quantidade de naterial no deposito
    pthread_mutex_t mutex; // Mutex para alterar as variaveis (zona critica)
} DepositoMaterial;

typedef struct {
    int material; // Quantidade de material na fabrica
    int canetas; // Quantidade de canetas na fabrica
    pthread_mutex_t mutexMaterial; // Mutex para alterar a variavel material
    pthread_mutex_t mutexCaneta; // Mutex para alterar a variavel Caneta
    sem_t semMaterial;  // Semaforo para indicar a quantidade de material na fabrica
    sem_t semCanetas; // Semáforo para indicar a quantidade de canetas disponiveis para enviar
} FabricaCaneta;

typedef struct {
    sem_t semControle; // Semáforo para fazer o controle da producao
} Controle;

typedef struct {
    int canetas; // Quantidade de canetas no deposito
    pthread_mutex_t mutex; // Mutex para alterar as variaveis (zona critica)
    sem_t empty; // Semáforo para indicar a quantidade de espacos vazio no deposito
    sem_t full;  // Semáforo para indicar a quantidade de canetas no deposito
} DepositoCaneta;

// Variáveis globais
DepositoMaterial depositoMaterial;
FabricaCaneta fabricaCaneta;
Controle structControle;
DepositoCaneta depositoCaneta;

int max_canetas; // Quantidade maxima de canetas que podem ser armazenadas no deposito

// Funções correspondentes as diferentes threads
void *deposito_material(void *arg){

    char **argv = (char **)arg;

    int qntEnviada = atoi(argv[2]);
    int tempoEnvio = atoi(argv[3]);

    int stop = 0;

    while (TRUE - stop) {
        //Controle

        pthread_mutex_lock(&depositoMaterial.mutex);
        pthread_mutex_lock(&fabricaCaneta.mutexMaterial);

        //Envia a quantidade disponivel de material
        if(depositoMaterial.material == 0){
            qntEnviada = 0;
            stop = 1;
            printf("Depósito de Material: Acabou a matéria-prima.\n");
        }
        else{
            if(depositoMaterial.material < qntEnviada){
                qntEnviada = depositoMaterial.material;
            }
            depositoMaterial.material -= qntEnviada;
            fabricaCaneta.material += qntEnviada;
            printf("Depósito de Material: Enviando %d unidades de matéria-prima.\n", qntEnviada);
        }

        pthread_mutex_unlock(&depositoMaterial.mutex);
        pthread_mutex_unlock(&fabricaCaneta.mutexMaterial);

        //Adiciona ao semaforo a quantidade enviada de material
        for(int i = 0; i < qntEnviada; i++){
            sem_post(&fabricaCaneta.semMaterial);
        }

        sleep(tempoEnvio);
    }

    return NULL;
}

void *fabrica_caneta(void *arg){

    char **argv = (char **)arg;

    int tempoFabricacao = atoi(argv[4]);

    int qntFabricada;

    while (TRUE) {
        //Controle

        //Verifica se tem materia prima para fabricar
        sem_wait(&fabricaCaneta.semMaterial);

        pthread_mutex_lock(&fabricaCaneta.mutexCaneta);
        pthread_mutex_lock(&fabricaCaneta.mutexMaterial);

        //Fabrica a/as canetas
        fabricaCaneta.canetas = fabricaCaneta.material;
        fabricaCaneta.material = 0;
        printf("Célula de fabricação de canetas: fabricou %d canetas", fabricaCaneta.canetas);

        pthread_mutex_unlock(&fabricaCaneta.mutexCaneta);
        pthread_mutex_unlock(&fabricaCaneta.mutexMaterial);

        //Adiciona no semaforo a quantidade de canetas que podem ser enviadas
        for(int i = 0; i < qntFabricada; i++) {
            sem_post(&fabricaCaneta.semCanetas);
        }

        sleep(tempoFabricacao * qntFabricada);
    }

    return NULL;
}

void *controle(){

    while(TRUE) {
        pthread_mutex_lock(&fabricaCaneta.mutexCaneta);
        pthread_mutex_lock(&depositoCaneta.mutex);
        if(depositoCaneta.canetas + fabricaCaneta.canetas >= max_canetas) {
            //bloqueia a fabrica de canetas e o deposito de material
        }
        if(depositoMaterial.material + fabricaCaneta.material + fabricaCaneta.canetas + depositoCaneta.canetas == 0) {
            //encerra o programa
        }
        pthread_mutex_unlock(&depositoCaneta.mutex);
        pthread_mutex_unlock(&fabricaCaneta.mutexCaneta);
    }

    return NULL;
}

void *deposito_caneta(){

    while (TRUE) {
        //Espera ter canetas disponiveis para serem enviadas
        sem_wait(&fabricaCaneta.semCanetas);

        //Checa se o deposito de caneta esta cheio
        sem_wait(&depositoCaneta.empty);

        pthread_mutex_lock(&fabricaCaneta.mutexCaneta);
        pthread_mutex_lock(&depositoCaneta.mutex);

        //Transfere uma caneta para o deposito
        fabricaCaneta.canetas--;
        depositoCaneta.canetas++;
        printf("Depósito de Canetas: Enviada 1 caneta. Estoque de canetas: %d\n", depositoCaneta.canetas);

        pthread_mutex_unlock(&depositoCaneta.mutex);
        pthread_mutex_unlock(&fabricaCaneta.mutexCaneta);

        //Adiciona uma caneta ao semaforo que mostra a quantidade de canetas do deposito
        sem_post(&depositoCaneta.full);
    }

    return NULL;
}

void *comprador(void *arg){

    char **argv = (char **)arg;

    int tempoEspera = atoi(argv[7]);
    int qntComprada;

    while (TRUE) {
        //Verifica se ha canetas disponiveis para a compra
        sem_wait(&depositoCaneta.full);

        pthread_mutex_lock(&depositoCaneta.mutex);

        qntComprada = atoi(argv[6]);

        //Se nao tem a quantidade suficiente compra o que tem no estoque
        if(qntComprada > depositoCaneta.canetas){
            qntComprada = depositoCaneta.canetas;
        }

        //Decresce o semaforo de acordo com a qnt comprada
        for(int i = 1; i < qntComprada; i++){
            sem_wait(&depositoCaneta.full);
        }

        //Compra as canetas
        depositoCaneta.canetas -= qntComprada;
        printf("Comprador: Comprou %d canetas.\n", qntComprada);

        pthread_mutex_unlock(&depositoCaneta.mutex);

        //Aumenta os espacos livres do deposito
        for(int i = 0; i < qntComprada; i++){
            sem_post(&depositoCaneta.empty);
        }

        sleep(tempoEspera);
    }

    return NULL;
}

int main(int argc, char *argv[]){
    //Lendo as variaveis do usuario
    if (argc != 8) {
        printf("Uso: %s <quantidade_inicial_deposito_material> <quantidade_enviada_material> <tempo_envio_material> <tempo_fabricacao> <quantidade_maxima_canetas> <quantidade_comprada> <tempo_espera_compra>\n", argv[0]);
        return 1;
    }

    // Inicializando as variáveis e semáforos

        //variaveis struct deposito material
    depositoMaterial.material = atoi(argv[1]);
    pthread_mutex_init(&depositoMaterial.mutex, NULL);

        //variaveis struct fabrica
    fabricaCaneta.material = 0;
    fabricaCaneta.canetas = 0;
    pthread_mutex_init(&fabricaCaneta.mutexMaterial, NULL);
    pthread_mutex_init(&fabricaCaneta.mutexCaneta, NULL);
    sem_init(&fabricaCaneta.semCanetas, 0, 0);
    sem_init(&fabricaCaneta.semMaterial, 0, 0);

        //variaveis struct controle
    sem_init(&structControle.semControle, 0, 1);

        //variaveis struct deposito caneta
    depositoCaneta.canetas = 0;
    pthread_mutex_init(&depositoCaneta.mutex, NULL);
    sem_init(&depositoCaneta.empty, 0, argv[5]);
    sem_init(&depositoCaneta.full, 0, 0);

    max_canetas = atoi(argv[5]);

    // Inicialização das threads
    pthread_t threads[5];
    pthread_create(&threads[0], NULL, deposito_material, (void *)argv);
    pthread_create(&threads[1], NULL, fabrica_caneta, (void *)argv);
    pthread_create(&threads[3], NULL, controle, NULL);
    pthread_create(&threads[4], NULL, deposito_caneta, NULL);
    pthread_create(&threads[5], NULL, comprador, (void *)argv);

    // Executa as threads
    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}