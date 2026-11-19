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
 * numeroAleatorio
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
 * compararInteiros
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
 * ordenarVetor
 * Ordena o vetor em ordem crescente com a qsort (stdlib.h).
 * Ela e rapida mesmo com 1.000.000 de elementos.
 */
void ordenarVetor(int v[], int n) {
    qsort(v, n, sizeof(int), compararInteiros);
}

/*
 * imprimirVetor
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

/* ============================================================================
 *  5. AREA DE IMPLEMENTACAO - 6 FUNCOES PARA ESCREVER
 * ============================================================================ */

/* ----------------------------------------------------------------------------
 *  5.1 VETORES
 *  O vetor e DINAMICO: criado com malloc, do tamanho que o usuario pediu,
 *  e liberado com free no fim da opcaoBuscar.
 * ---------------------------------------------------------------------------- */

/*
 * criarVetorAleatorio                                           [X] FEITO
 * Dificuldade: facil
 *
 * O que faz: aloca um vetor de n inteiros e preenche com numeros sorteados
 *            entre VALOR_MINIMO e VALOR_MAXIMO.
 * Variaveis locais: int *v;  int i;
 * Passos:
 *   1. v = (int *) malloc(n * sizeof(int)).
 *   2. Se v == NULL (sem memoria), devolver NULL.
 *   3. Para i de 0 a n - 1:
 *        v[i] = numeroAleatorio(VALOR_MINIMO, VALOR_MAXIMO).
 *   4. Devolver v.
 * Retorno: o endereco do vetor criado; NULL se faltou memoria.
 * Chamada por: opcaoBuscar.
 */
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

/*
 * copiarVetor                                                   [X] FEITO
 * Dificuldade: facil
 *
 * O que faz: cria um vetor NOVO, do mesmo tamanho, com os mesmos valores.
 *            A copia e que vai ser ordenada para a busca binaria; o
 *            original fica como esta para a busca sequencial.
 * Variaveis locais: int *copia;  int i;
 * Passos:
 *   1. copia = (int *) malloc(n * sizeof(int)).
 *   2. Se copia == NULL, devolver NULL.
 *   3. Para i de 0 a n - 1: copia[i] = origem[i].
 *   4. Devolver copia.
 *
 *   ATENCAO: "copia = origem" NAO copia o vetor. So copia o ENDERECO:
 *   os dois ponteiros apontariam para o mesmo vetor e, ao ordenar a
 *   "copia", o original tambem ficaria ordenado.
 *
 * Retorno: o endereco da copia; NULL se faltou memoria.
 * Chamada por: opcaoBuscar.
 */
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


/* ----------------------------------------------------------------------------
 *  5.2 BUSCAS
 *  As duas devolvem a POSICAO onde acharam a chave, ou NAO_ENCONTRADO (-1).
 *  O numero de comparacoes volta pelo ponteiro "comparacoes" (passagem
 *  por referencia), porque a funcao ja usa o retorno para a posicao.
 * ---------------------------------------------------------------------------- */

/*
 * buscaSequencial                                               [X] FEITO
 * Dificuldade: facil
 *
 * O que faz: procura a chave olhando os elementos um por um, do primeiro
 *            ao ultimo. Funciona com o vetor fora de ordem.
 * Variaveis locais: int i;
 * Passos:
 *   1. *comparacoes = 0.
 *   2. Para i de 0 a n - 1:
 *        (*comparacoes)++;            (conta a comparacao da linha abaixo)
 *        se v[i] == chave: devolver i.
 *   3. Se o laco terminou, a chave nao esta no vetor:
 *        devolver NAO_ENCONTRADO.
 *
 *   ATENCAO aos parenteses em (*comparacoes)++. Sem eles, "*comparacoes++"
 *   anda com o PONTEIRO em vez de somar 1 no valor.
 *
 * Exemplo: v = {40, 15, 72, 8}, chave = 72
 *     i = 0: 40 == 72? nao     comparacoes = 1
 *     i = 1: 15 == 72? nao     comparacoes = 2
 *     i = 2: 72 == 72? SIM     comparacoes = 3  -> devolve 2
 *
 * Retorno: a posicao da chave; NAO_ENCONTRADO se nao achou.
 * Chamada por: opcaoBuscar.
 */
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

/*
 * buscaBinaria                                                  [X] FEITO
 * Dificuldade: media
 *
 * O que faz: procura a chave num vetor ORDENADO, sempre olhando o meio
 *            do pedaco que ainda pode ter a chave e descartando a outra
 *            metade.
 * Variaveis locais: int inicio = 0, fim = n - 1, meio;
 * Passos:
 *   1. *comparacoes = 0.
 *   2. ENQUANTO inicio <= fim:
 *        meio = (inicio + fim) / 2;
 *        (*comparacoes)++;
 *        se v[meio] == chave: devolver meio.          (achou)
 *        (*comparacoes)++;
 *        se chave < v[meio]: fim = meio - 1;          (fica com a esquerda)
 *        senao:              inicio = meio + 1;       (fica com a direita)
 *   3. Se saiu do laco (inicio passou do fim), nao esta no vetor:
 *        devolver NAO_ENCONTRADO.
 *
 * Exemplo: v = {8, 15, 23, 40, 51, 72, 90}, chave = 72
 *     inicio fim meio  v[meio]  comparacoes
 *       0     6    3     40     40 == 72? nao (1); 72 < 40? nao (2)
 *                               -> inicio = 4
 *       4     6    5     72     72 == 72? SIM (3) -> devolve 5
 *
 * Retorno: a posicao da chave no vetor ORDENADO; NAO_ENCONTRADO se nao
 *          achou.
 * Chamada por: opcaoBuscar.
 */
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


/* ----------------------------------------------------------------------------
 *  5.3 RESULTADO E OPCAO DO MENU
 * ---------------------------------------------------------------------------- */

/*
 * imprimirResultado                                             [X] FEITO
 * Dificuldade: facil
 *
 * O que faz: mostra o resultado de UM numero buscado.
 *   "posicao" e o retorno da busca sequencial: diz se achou ou nao.
 *   (As duas buscas sempre concordam se o numero esta ou nao no vetor.)
 * Passos:
 *   1. Se posicao != NAO_ENCONTRADO:
 *        printf("\nNumero %d: encontra-se no conjunto de dados.\n", numero);
 *      Senao:
 *        printf("\nNumero %d: nao encontra-se no conjunto de dados.\n", numero);
 *   2. printf("  Comparacoes na busca sequencial: %d\n", compSequencial);
 *   3. printf("  Comparacoes na busca binaria:    %d\n", compBinaria);
 *
 * Saida esperada:
 *     Numero 532: encontra-se no conjunto de dados.
 *       Comparacoes na busca sequencial: 37
 *       Comparacoes na busca binaria:    11
 *
 * Chamada por: opcaoBuscar.
 */
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

/*
 * opcaoBuscar                                                   [ ] A FAZER
 * Dificuldade: media
 *
 * O que faz: o fluxo inteiro da parte 2 do enunciado.
 * Variaveis locais: int n, i;
 *                   int *original, *ordenado;
 *                   int numeros[QTD_BUSCAS];
 *                   int posicao, compSequencial, compBinaria;
 *                   char mensagem[40];
 * Passos:
 *   1. n = lerInteiro("\nQuantidade de elementos do conjunto: ").
 *      Se n <= 0: printf("A quantidade deve ser maior que zero.\n") e
 *      return.
 *   2. original = criarVetorAleatorio(n).
 *      Se NULL: printf("Memoria insuficiente.\n") e return.
 *   3. ordenado = copiarVetor(original, n).
 *      Se NULL: free(original), printf("Memoria insuficiente.\n") e return.
 *   4. ordenarVetor(ordenado, n).
 *   5. imprimirVetor("Vetor original", original, n);
 *      imprimirVetor("Vetor ordenado", ordenado, n);
 *      (so aparecem se n <= LIMITE_IMPRESSAO)
 *   6. Ler os 3 numeros. Para i de 0 a QTD_BUSCAS - 1:
 *        sprintf(mensagem, "%do numero a buscar: ", i + 1);
 *        numeros[i] = lerInteiro(mensagem);
 *   7. printf("\n===== RESULTADO =====\n");
 *      Para i de 0 a QTD_BUSCAS - 1:
 *        posicao = buscaSequencial(original, n, numeros[i], &compSequencial);
 *        buscaBinaria(ordenado, n, numeros[i], &compBinaria);
 *        imprimirResultado(numeros[i], posicao, compSequencial, compBinaria);
 *      (da binaria so se usa o numero de comparacoes: a posicao que ela
 *       devolve e a do vetor ORDENADO, e "encontra-se ou nao" a
 *       sequencial ja respondeu.)
 *   8. free(original); free(ordenado);   (liberar os DOIS vetores)
 *
 *   Por que DOIS vetores? O enunciado manda: a sequencial e feita no
 *   vetor original nao ordenado, e a binaria no vetor ordenado. Se
 *   ordenasse o original, a sequencial deixaria de ser no vetor original.
 *
 * Chamada por: main (opcao 1).
 */
void opcaoBuscar(void) {
    /* TODO: implementar */
    printf("[opcaoBuscar ainda nao implementada]\n");
}
