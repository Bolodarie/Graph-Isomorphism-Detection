# Graph-Isomorphism-Detection
# Detecção de Isomorfismo de Grafos: Abordagem Paralela com OpenMP

## 1. Resumo
Este repositório contém uma implementação em linguagem C para a resolução exata do problema de Isomorfismo de Grafos. Devido à natureza combinatória do problema, cuja verificação tem complexidade temporal no pior caso de $O(V!)$, a solução emprega um algoritmo de *backtracking* otimizado por paralelismo de tarefas (*Task Parallelism*) utilizando a API OpenMP.

## 2. Metodologia e Otimizações
A arquitetura da solução foi projetada com foco em alto desempenho computacional:

* **Estruturas de Dados:** Utilização de matrizes de adjacência para garantir localidade de cache e tempo de acesso em $O(1)$ durante as validações de arestas.
* **Granularidade do Paralelismo:** Aplicação da diretiva `#pragma omp task` restrita aos níveis iniciais da árvore de recursão. Esta abordagem mitiga o *overhead* de criação de tarefas enquanto garante distribuição adequada de carga entre as *threads*.
* **Poda Global (*Early Exit*):** Implementação de variáveis de estado compartilhadas com sincronização de memória (`#pragma omp flush`) para interrupção imediata de ramificações de busca assim que uma bijeção válida é identificada.
* **Isolamento de Contexto:** Uso de cópias rasas (`memcpy`) diretamente na *stack* de execução para manter a integridade das permutações em cada *thread*, eliminando condições de corrida (*data races*).

## 3. Requisitos e Compilação
O projeto requer um compilador com suporte nativo ao padrão OpenMP (GCC recomendado).

Para compilar o código fonte, aplique a flag de paralelismo e a *flag* de otimização máxima (`-O3`):
```bash
gcc -O3 -fopenmp isomorfismo.c -o isomorfismo
```

## 4. Execução
O programa lê os parâmetros a partir da entrada padrão (`stdin`). Para testes em lote, recomenda-se o redirecionamento de arquivos:

```bash
./isomorfismo < input.txt
```

O grau de paralelismo pode ser estipulado em tempo de execução através da variável de ambiente `OMP_NUM_THREADS`:
```bash
OMP_NUM_THREADS=8 ./isomorfismo < input.txt
```

## 5. Especificação de I/O
### Entrada
* Inteiros $V_g$ e $E_g$, denotando os vértices e arestas do grafo $G$, seguidos por $E_g$ pares de coordenadas $(u, v)$ especificando suas arestas.
* Inteiros $V_h$ e $E_h$, denotando os vértices e arestas do grafo $H$, seguidos por $E_h$ pares de coordenadas $(u, v)$ especificando suas arestas.

### Saída
* **Caso Isomorfo:** Uma linha com inteiros separados por espaço, representando a projeção $f(u) = v$.
* **Caso Não Isomorfo:** A constante textual `The graphs are not isomorphic`.
