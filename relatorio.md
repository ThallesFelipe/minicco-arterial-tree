# Relatorio - MiniCCO-0

## Descricao da implementacao

O programa foi implementado em C no arquivo `minicco.c`. A arvore arterial simplificada usa as estruturas `Point`, `Segment` e `No`, conforme sugerido no enunciado. Cada no armazena o ponto geometrico, o identificador inteiro, ponteiros para os filhos esquerdo e direito, e ponteiro para o pai.

A raiz e criada na origem `(0, 0)`. Em seguida, o programa gera `Nterm` pontos aleatorios dentro de um dominio circular de raio `R`. Para cada novo ponto, a implementacao percorre recursivamente a arvore e gera candidatos entre os nos que ainda possuem menos de dois filhos. Cada candidato recebe como custo a distancia euclidiana entre o novo ponto e o ponto de conexao. Os candidatos sao ordenados com `qsort`, e a primeira conexao valida e inserida na arvore.

A validacao geometrica verifica tres condicoes: o ponto deve estar dentro do dominio circular, o novo segmento nao pode cruzar segmentos ja existentes da arvore e a distancia minima ate segmentos nao incidentes deve ser maior que `epsilon`. O valor usado foi `epsilon = 0.01 * R`, mantendo a escala da restricao proporcional ao tamanho do dominio. Segmentos incidentes ao no de conexao sao ignorados nessa verificacao, pois compartilham naturalmente o ponto de bifurcacao.

Ao final, o programa imprime o numero total de nos, o numero de folhas, o comprimento total da arvore e o numero de conexoes rejeitadas. Os segmentos sao exportados no arquivo `segments.csv`, com as colunas `parent_id`, `child_id`, `x1`, `y1`, `x2` e `y2`.

As principais funcoes geometricas implementadas foram `distancia_pontos`, `orientacao`, `segmentos_intersectam`, `distancia_ponto_segmento` e `ponto_no_dominio`. A intersecao de segmentos usa o sinal da orientacao para detectar cruzamentos proprios e tambem trata casos colineares com a funcao `ponto_em_segmento`. A distancia entre dois segmentos e calculada a partir das distancias ponto-segmento quando nao ha intersecao.

## Visualizacao

O script `visualizar.py` carrega o arquivo `segments.csv` usando `csv.DictReader`, monta os pontos e linhas no formato esperado pelo PyVista e exibe a arvore com `pv.PolyData`. A execucao basica e:

```bash
python3 visualizar.py segments.csv
```

## Exemplos de execucao

Compilacao:

```bash
make
```

Execucao:

```bash
./programa 100 10.0
```

## Analise dos resultados

Os resultados dependem da semente aleatoria usada em cada execucao. Em geral, o numero total de nos deve ser `Nterm + 1`, pois a raiz inicial tambem e contada. O numero de folhas depende da forma final da arvore, ja que um ponto inicialmente terminal pode receber novos filhos em iteracoes posteriores. O comprimento total tende a crescer conforme `Nterm` aumenta, porque mais segmentos sao adicionados ao dominio.

As conexoes rejeitadas aparecem quando um candidato viola alguma restricao geometrica. Como os candidatos sao testados em ordem crescente de custo, conexoes mais curtas sao priorizadas, mas ainda podem ser rejeitadas por intersecao ou proximidade excessiva de outro segmento.

Nos testes executados, o crescimento do comprimento total foi compativel com o aumento de pontos terminais. A quantidade de conexoes rejeitadas tambem aumentou para `Nterm = 50` e `Nterm = 100`, o que e esperado, pois uma arvore mais densa oferece mais segmentos contra os quais cada nova conexao precisa ser validada.

## Complexidade

A busca de candidatos percorre todos os nos da arvore para cada ponto novo. Se a arvore possui `n` nos, essa etapa tem custo `O(n)`. Depois disso, os candidatos sao ordenados, com custo `O(n log n)` no pior caso.

O teste de intersecao e distancia compara cada conexao candidata com os segmentos ja existentes. Como ha `O(n)` segmentos na arvore, a validacao de uma candidata custa `O(n)`. No pior caso, varios candidatos podem ser rejeitados ate que uma conexao valida seja encontrada, levando a um custo `O(n^2)` por ponto inserido. Essa abordagem e simples e adequada para o tamanho pedido no exercicio, mas poderia ser otimizada futuramente com estruturas espaciais.

## Testes executados

Os testes foram executados no WSL com raio `R = 10.0`.

| Escala  | Comando              | Resultado |
|---------|----------------------|-----------|
| Pequeno | `./programa 10 10.0` | 11 nos, 5 folhas, comprimento total 35.576768, 0 conexoes rejeitadas |
| Medio   | `./programa 50 10.0` | 51 nos, 19 folhas, comprimento total 146.036811, 45 conexoes rejeitadas |
| Grande  | `./programa 100 10.0` | 101 nos, 39 folhas, comprimento total 241.773927, 250 conexoes rejeitadas |

Os arquivos exportados desses testes foram salvos em:

- `resultados/segments_10.csv`
- `resultados/segments_50.csv`
- `resultados/segments_100.csv`
