/*
 * ============================================================================
 *  3o TRABALHO PRATICO - METODOS DE ORDENACAO E BUSCA
 *  (Estrutura de Dados - UFPR/TADS)
 *  Parte 2: Busca de Dados - Pesquisa Sequencial e Pesquisa Binaria.
 *
 *  Equipe: Vitor Savi do Nascimento
 *          Felipe Fernandes de Azevedo
 * ============================================================================
 *
 *  ALGORITMOS
 *    1. Pesquisa Sequencial: compara a chave com v[0], v[1], v[2]... ate
 *       achar ou ate o vetor acabar. Funciona em qualquer vetor.
 *       Melhor caso: 1 comparacao (a chave esta em v[0]).
 *       Pior caso: n comparacoes (esta no fim ou nao esta no vetor).
 *    2. Pesquisa Binaria: so funciona em vetor ORDENADO. Olha o elemento
 *       do MEIO: se e a chave, achou; se a chave e menor, descarta a
 *       metade da direita; se e maior, descarta a da esquerda. Repete na
 *       metade que sobrou. A cada passo, metade do vetor e descartada.
 *       Com 1.000.000 de elementos sao no maximo 20 passos (2^20 e mais
 *       de 1 milhao).
 *
 *  CONTAGEM DE COMPARACOES (decisao de projeto)
 *    Conta-se toda comparacao entre a CHAVE e um ELEMENTO DO VETOR. As
 *    comparacoes de controle do laco (i < n, inicio <= fim) nao contam.
 *    - Sequencial: 1 comparacao por elemento olhado (v[i] == chave).
 *    - Binaria: em cada passo, 1 comparacao (v[meio] == chave) e, se nao
 *      achou, mais 1 (chave < v[meio]). Por isso, com 1.000.000 de
 *      elementos, a binaria faz no maximo 40 comparacoes (20 passos x 2).
 *
 *
 *  REFERENCIAS
 *    - ZIVIANI, N. Projeto de Algoritmos com Implementacoes em Pascal e C.
 *      3. ed. Sao Paulo: Cengage Learning, 2011. (Cap. 5, Pesquisa em
 *      Memoria Primaria)
 *    - TENENBAUM, A. M.; LANGSAM, Y.; AUGENSTEIN, M. J. Estruturas de
 *      Dados Usando C. Sao Paulo: Makron Books, 1995.
 *
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>


#define VALOR_MINIMO      100      /* menor numero sorteado                  */
#define VALOR_MAXIMO      100000   /* maior numero sorteado                  */
#define QTD_BUSCAS        3        /* quantos numeros o usuario busca        */
#define LIMITE_IMPRESSAO  50       /* vetores ate esse tamanho sao impressos */
#define NAO_ENCONTRADO    -1       /* retorno das buscas quando nao acha     */


/* --- Funcoes de Apoio --- */
void limparBuffer(void);
int  lerInteiro(char mensagem[]);
void exibirMenu(void);
int  numeroAleatorio(int minimo, int maximo);
int  compararInteiros(const void *a, const void *b);
void ordenarVetor(int v[], int n);
void imprimirVetor(char titulo[], int v[], int n);

/* --- Vetores --- */
int *criarVetorAleatorio(int n);
int *copiarVetor(int origem[], int n);

/* --- Buscas --- */
int  buscaSequencial(int v[], int n, int chave, int *comparacoes);
int  buscaBinaria(int v[], int n, int chave, int *comparacoes);

/* --- Resultado e opcao do menu --- */
void imprimirResultado(int numero, int posicao, int compSequencial, int compBinaria);
void opcaoBuscar(void);


int main(void) {
    int opcao;

    srand((unsigned) time(NULL));       /* sorteios diferentes a cada execucao */

    do {
        exibirMenu();
        opcao = lerInteiro("Opcao: ");
        if (feof(stdin)) {
            opcao = 2;                  /* entrada acabou: encerra */
        }

        switch (opcao) {
            case 1:
                opcaoBuscar();
                break;
            case 2:
                printf("Ate a proxima!\n");
                break;
            default:
                printf("Opcao invalida. Digite 1 ou 2.\n");
        }
    } while (opcao != 2);

    return 0;
}


void limparBuffer(void) {
    int c;

    do {
        c = getchar();
    } while (c != '\n' && c != EOF);
}


int lerInteiro(char mensagem[]) {
    char linha[64];
    char *fim;
    long valor;
    size_t tamanho;

    while (1) {
        printf("%s", mensagem);
        if (fgets(linha, sizeof linha, stdin) == NULL) {
            return 0;                           /* fim da entrada */
        }
        tamanho = strlen(linha);
        if (tamanho > 0 && linha[tamanho - 1] == '\n') {
            linha[tamanho - 1] = '\0';          /* tira o Enter */
        } else if (!feof(stdin)) {
            limparBuffer();                     /* linha grande demais */
            printf("Valor invalido. Digite um numero inteiro.\n");
            continue;
        }

        valor = strtol(linha, &fim, 10);
        while (*fim == ' ') {                   /* aceita espacos no fim */
            fim++;
        }
        if (fim != linha && *fim == '\0' && valor >= INT_MIN && valor <= INT_MAX) {
            return (int) valor;
        }
        printf("Valor invalido. Digite um numero inteiro.\n");
    }
}


void exibirMenu(void) {
    printf("\n===== BUSCA DE DADOS =====\n");
    printf("1. Realizar busca\n");
    printf("2. Sair\n");
}

/*
 * Sorteia um inteiro entre "minimo" e "maximo" (os dois inclusive).
 *
 * Por que nao so rand() % 99901 + 100? Porque no Windows a rand() devolve
 * no maximo 32.767 (RAND_MAX). Os numeros acima disso nunca seriam
 * sorteados. Aqui dois rand() sao combinados num numero bem maior
 * (como os dois digitos de um numero de dois algarismos), e o resto da
 * divisao o coloca na faixa pedida. Usa long long para nao estourar.
 */
int numeroAleatorio(int minimo, int maximo) {
    long long grande;

    grande = (long long) rand() * ((long long) RAND_MAX + 1) + rand();
    return minimo + (int) (grande % (maximo - minimo + 1));
}

/*
 * Funcao que a qsort usa para saber a ordem de dois elementos.
 * Devolve negativo se a < b, zero se iguais, positivo se a > b.
 * (Nao usa "x - y" porque a subtracao pode estourar o int.)
 */
int compararInteiros(const void *a, const void *b) {
    int x = *(const int *) a;
    int y = *(const int *) b;

    if (x < y) {
        return -1;
    }
    if (x > y) {
        return 1;
    }
    return 0;
}

/*
 * Ordena o vetor em ordem crescente com a qsort (stdlib.h).
 * Ela e rapida mesmo com 1.000.000 de elementos.
 */
void ordenarVetor(int v[], int n) {
    qsort(v, n, sizeof(int), compararInteiros);
}

/*
 * Mostra o vetor, 10 numeros por linha, so se ele for pequeno
 * (ate LIMITE_IMPRESSAO elementos). Serve para conferir as buscas.
 */
void imprimirVetor(char titulo[], int v[], int n) {
    int i;

    if (n > LIMITE_IMPRESSAO) {
        return;
    }
    printf("\n%s:\n", titulo);
    for (i = 0; i < n; i++) {
        printf("%7d", v[i]);
        if ((i + 1) % 10 == 0 || i == n - 1) {
            printf("\n");
        }
    }
}


int *criarVetorAleatorio(int n) { 
    int *v, i;

    v = (int *) malloc(n * sizeof(int));
    if (v == NULL) {
        return NULL;
    }

    for (i = 0; i < n; i++) {
        v[i] = numeroAleatorio(VALOR_MINIMO, VALOR_MAXIMO);
    }

    return v;
} 


int *copiarVetor(int origem[], int n) { 
    int *copia, i;

    copia = (int *) malloc(n * sizeof(int));
    if (copia == NULL) {
        return NULL;
    }

    for (i = 0; i < n; i++) {
        copia[i] = origem[i];
    }

    return copia;
} 


int buscaSequencial(int v[], int n, int chave, int *comparacoes) { 
    int i;

    *comparacoes = 0; 

    for (i = 0; i < n; i++) {
        (*comparacoes)++;
        if (v[i] == chave) {
            return i;
        }
    }

    return NAO_ENCONTRADO; 
} 


int buscaBinaria(int v[], int n, int chave, int *comparacoes) { 
    int inicio = 0;
    int fim = n - 1;
    int meio;

    *comparacoes = 0; 

    while (inicio <= fim) {
        meio = (inicio + fim) / 2;
        
        (*comparacoes)++;
        if (v[meio] == chave) {
            return meio;
        }

        (*comparacoes)++;
        if (chave < v[meio]) {
            fim = meio - 1;
        }
        else {
            inicio = meio + 1;
        }
    }

    return NAO_ENCONTRADO; 
    
} 


void imprimirResultado(int numero, int posicao, int compSequencial, int compBinaria) { 
    if (posicao != NAO_ENCONTRADO) {
        printf("\nNumero %d: encontra-se no conjunto de dados.\n", numero);
    }
    else {
        printf("\nNumero %d: nao encontra-se no conjunto de dados.\n", numero);
    }

    printf("  Comparacoes na busca sequencial: %d\n", compSequencial);
    printf("  Comparacoes na busca binaria:    %d\n", compBinaria);
} 


void opcaoBuscar(void) { 
    int n, i;
    int *original, *ordenado;
    int numeros[QTD_BUSCAS];
    int posicao, compSequencial, compBinaria;
    char mensagem[40];

    n = lerInteiro("\nDigite a quantidade de elementos do conjunto: ");
    if (n <= 0) {
        printf("A quantidade deve ser maior que zero.\n");
        return;
    }

    original = criarVetorAleatorio(n);
    if (original == NULL) {
        printf("Memoria insuficiente!!\n");
        return;
    }

    ordenado = copiarVetor(original, n);
    if (ordenado == NULL) {
        free(original);
        printf("Memoria insuficiente!!\n");
        return;
    }

    ordenarVetor(ordenado, n);
    // so aparecem se n <= LIMITE_IMPRESSAO
    imprimirVetor("Vetor original", original, n);
    imprimirVetor("Vetor ordenado", ordenado, n);

    for (i = 0; i < QTD_BUSCAS; i++) {
        sprintf(mensagem, "%do numero a buscar: ", i + 1);
        numeros[i] = lerInteiro(mensagem);
    }

    printf("\n===== RESULTADO =====\n");
    for (i = 0; i < QTD_BUSCAS; i++) {
        posicao = buscaSequencial(original, n, numeros[i], &compSequencial);
        buscaBinaria(ordenado, n, numeros[i], &compBinaria);
        imprimirResultado(numeros[i], posicao, compSequencial, compBinaria);
    }

    free(original);
    free(ordenado);

} 
