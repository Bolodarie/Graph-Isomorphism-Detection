#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <omp.h>

#define MAX_V 100

int Vg, Eg, Vh, Eh;
int adjG[MAX_V][MAX_V];
int adjH[MAX_V][MAX_V];

// Variáveis compartilhadas para controlar a concorrência
bool found_iso = false;
int solution[MAX_V];

// Função para verificar se a permutação atual preserva as adjacências
bool check_isomorphism(const int *p) {
    for (int i = 0; i < Vg; ++i) {
        for (int j = 0; j < Vg; ++j) {
            // Se a aresta em G não tem correspondência exata no mapeamento em H
            if (adjG[i][j] != adjH[p[i]][p[j]]) {
                return false;
            }
        }
    }
    return true;
}

// Backtracking para gerar permutações e validar
void find_isomorphism(int depth, const int *current_p, const bool *used) {
    // Sincroniza a leitura da flag. Se outra thread já achou a solução, aborta este galho.
    #pragma omp flush(found_iso)
    if (found_iso) return;

    // Caso base: permutação completa gerada
    if (depth == Vg) {
        if (check_isomorphism(current_p)) {
            // Região crítica para evitar condição de corrida na gravação da resposta
            #pragma omp critical
            {
                if (!found_iso) {
                    found_iso = true;
                    // Copia o resultado final para o array global
                    for (int i = 0; i < Vg; i++) {
                        solution[i] = current_p[i];
                    }
                }
            }
        }
        return;
    }

    // Gera os próximos nós da árvore de permutações
    for (int i = 0; i < Vh; ++i) {
        if (!used[i]) {
            // Alocação na stack para garantir que cada thread tenha seu próprio estado.
            // O limite de 100 vértices torna essa abordagem eficiente sem risco de Stack Overflow.
            int next_p[MAX_V];
            bool next_used[MAX_V];
            
            memcpy(next_p, current_p, Vg * sizeof(int));
            memcpy(next_used, used, Vh * sizeof(bool));
            
            next_p[depth] = i;
            next_used[i] = true;

            // Criamos tarefas OpenMP apenas para os primeiros níveis da árvore
            if (depth < 2) {
                #pragma omp task shared(found_iso, solution)
                {
                    find_isomorphism(depth + 1, next_p, next_used);
                }
            } else {
                // Para níveis mais profundos, segue sequencial na thread atual
                find_isomorphism(depth + 1, next_p, next_used);
            }
        }
    }
}

int main() {
    // Leitura do Grafo G
    if (scanf("%d", &Vg) != 1) return 0;
    scanf("%d", &Eg);

    memset(adjG, 0, sizeof(adjG));
    for (int i = 0; i < Eg; ++i) {
        int u, v;
        scanf("%d %d", &u, &v);
        adjG[u][v] = 1;
        adjG[v][u] = 1; // Grafos não direcionados
    }

    // Leitura do Grafo H
    scanf("%d %d", &Vh, &Eh);
    memset(adjH, 0, sizeof(adjH));
    for (int i = 0; i < Eh; ++i) {
        int u, v;
        scanf("%d %d", &u, &v);
        adjH[u][v] = 1;
        adjH[v][u] = 1; // Grafos não direcionados
    }

    // Poda imediata: Se o número de vértices ou arestas for diferente, é impossível ser isomorfo
    if (Vg != Vh || Eg != Eh) {
        printf("The graphs are not isomorphic\n");
        return 0;
    }

    // Estado inicial
    int initial_p[MAX_V];
    bool initial_used[MAX_V];
    memset(initial_p, -1, sizeof(initial_p));
    memset(initial_used, false, sizeof(initial_used));

    // Inicializa a região paralela
    #pragma omp parallel
    {
        // Apenas uma thread dispara o nó raiz da árvore
        #pragma omp single
        {
            find_isomorphism(0, initial_p, initial_used);
        }
        // Aguarda todas as tarefas filhas finalizarem
        #pragma omp taskwait
    }

    // Impressão da saída
    if (found_iso) {
        for (int i = 0; i < Vg; ++i) {
            printf("%d%s", solution[i], (i == Vg - 1) ? "" : " ");
        }
        printf("\n");
    } else {
        printf("The graphs are not isomorphic\n");
    }

    return 0;
}
