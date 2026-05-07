#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define EPS 1e-9
#define DISTANCE_EPSILON_FACTOR 0.01
#define MAX_POINT_ATTEMPTS 10000
#define OUTPUT_FILE "segments.csv"

typedef struct {
  double x, y;
} Point;

typedef struct {
  Point a, b;
} Segment;

typedef struct No {
  struct No *esq;
  struct No *dir;
  struct No *pai;
  Point p;
  int id;
} No;

typedef No *ptrNo;

typedef struct {
  ptrNo no;
  double custo;
} Candidate;

static double distancia_pontos(Point a, Point b) {
  double dx = b.x - a.x;
  double dy = b.y - a.y;
  return sqrt(dx * dx + dy * dy);
}

static double orientacao(Point a, Point b, Point c) {
  return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

static int sinal(double valor) {
  if (valor > EPS) {
    return 1;
  }

  if (valor < -EPS) {
    return -1;
  }

  return 0;
}

static bool ponto_em_segmento(Point a, Point b, Point p) {
  return p.x >= fmin(a.x, b.x) - EPS && p.x <= fmax(a.x, b.x) + EPS &&
         p.y >= fmin(a.y, b.y) - EPS && p.y <= fmax(a.y, b.y) + EPS &&
         sinal(orientacao(a, b, p)) == 0;
}

static bool segmentos_intersectam(Segment s1, Segment s2) {
  double o1 = orientacao(s1.a, s1.b, s2.a);
  double o2 = orientacao(s1.a, s1.b, s2.b);
  double o3 = orientacao(s2.a, s2.b, s1.a);
  double o4 = orientacao(s2.a, s2.b, s1.b);

  int so1 = sinal(o1);
  int so2 = sinal(o2);
  int so3 = sinal(o3);
  int so4 = sinal(o4);

  if (so1 * so2 < 0 && so3 * so4 < 0) {
    return true;
  }

  if (so1 == 0 && ponto_em_segmento(s1.a, s1.b, s2.a)) {
    return true;
  }

  if (so2 == 0 && ponto_em_segmento(s1.a, s1.b, s2.b)) {
    return true;
  }

  if (so3 == 0 && ponto_em_segmento(s2.a, s2.b, s1.a)) {
    return true;
  }

  if (so4 == 0 && ponto_em_segmento(s2.a, s2.b, s1.b)) {
    return true;
  }

  return false;
}

static double distancia_ponto_segmento(Point p, Segment s) {
  double dx = s.b.x - s.a.x;
  double dy = s.b.y - s.a.y;
  double comprimento_quadrado = dx * dx + dy * dy;

  if (comprimento_quadrado <= EPS) {
    return distancia_pontos(p, s.a);
  }

  double t = ((p.x - s.a.x) * dx + (p.y - s.a.y) * dy) / comprimento_quadrado;

  if (t < 0.0) {
    t = 0.0;
  } else if (t > 1.0) {
    t = 1.0;
  }

  Point projecao = {s.a.x + t * dx, s.a.y + t * dy};

  return distancia_pontos(p, projecao);
}

static double distancia_segmentos(Segment s1, Segment s2) {
  double d1;
  double d2;
  double d3;
  double d4;

  if (segmentos_intersectam(s1, s2)) {
    return 0.0;
  }

  d1 = distancia_ponto_segmento(s1.a, s2);
  d2 = distancia_ponto_segmento(s1.b, s2);
  d3 = distancia_ponto_segmento(s2.a, s1);
  d4 = distancia_ponto_segmento(s2.b, s1);

  return fmin(fmin(d1, d2), fmin(d3, d4));
}

static bool ponto_no_dominio(Point p, double raio) {
  return p.x * p.x + p.y * p.y <= raio * raio + EPS;
}

static double numero_aleatorio(double minimo, double maximo) {
  double t = (double)rand() / (double)RAND_MAX;
  return minimo + t * (maximo - minimo);
}

static Point gerar_ponto_no_dominio(double raio) {
  Point p;

  do {
    p.x = numero_aleatorio(-raio, raio);
    p.y = numero_aleatorio(-raio, raio);
  } while (!ponto_no_dominio(p, raio));

  return p;
}

static ptrNo criar_no(int id, Point p) {
  ptrNo no = malloc(sizeof(No));

  if (no == NULL) {
    return NULL;
  }

  no->esq = NULL;
  no->dir = NULL;
  no->pai = NULL;
  no->p = p;
  no->id = id;

  return no;
}

static int quantidade_filhos(ptrNo no) {
  int quantidade = 0;

  if (no->esq != NULL) {
    quantidade++;
  }

  if (no->dir != NULL) {
    quantidade++;
  }

  return quantidade;
}

static bool inserir_no(ptrNo pai, ptrNo filho) {
  if (pai->esq == NULL) {
    pai->esq = filho;
  } else if (pai->dir == NULL) {
    pai->dir = filho;
  } else {
    return false;
  }

  filho->pai = pai;
  return true;
}

static int contar_nos(ptrNo raiz) {
  if (raiz == NULL) {
    return 0;
  }

  return 1 + contar_nos(raiz->esq) + contar_nos(raiz->dir);
}

static int contar_folhas(ptrNo raiz) {
  if (raiz == NULL) {
    return 0;
  }

  if (raiz->esq == NULL && raiz->dir == NULL) {
    return 1;
  }

  return contar_folhas(raiz->esq) + contar_folhas(raiz->dir);
}

static double comprimento_total(ptrNo raiz) {
  double total = 0.0;

  if (raiz == NULL) {
    return 0.0;
  }

  if (raiz->esq != NULL) {
    total += distancia_pontos(raiz->p, raiz->esq->p);
    total += comprimento_total(raiz->esq);
  }

  if (raiz->dir != NULL) {
    total += distancia_pontos(raiz->p, raiz->dir->p);
    total += comprimento_total(raiz->dir);
  }

  return total;
}

static void preencher_candidatos(ptrNo raiz, Point novo_ponto,
                                 Candidate *candidatos, int *quantidade) {
  if (raiz == NULL) {
    return;
  }

  if (quantidade_filhos(raiz) < 2) {
    candidatos[*quantidade].no = raiz;
    candidatos[*quantidade].custo = distancia_pontos(raiz->p, novo_ponto);
    (*quantidade)++;
  }

  preencher_candidatos(raiz->esq, novo_ponto, candidatos, quantidade);
  preencher_candidatos(raiz->dir, novo_ponto, candidatos, quantidade);
}

static int comparar_candidatos(const void *a, const void *b) {
  const Candidate *c1 = a;
  const Candidate *c2 = b;

  if (c1->custo < c2->custo) {
    return -1;
  }

  if (c1->custo > c2->custo) {
    return 1;
  }

  return 0;
}

static bool segmento_incidente_ao_no(Segment segmento, ptrNo no) {
  return (fabs(segmento.a.x - no->p.x) <= EPS &&
          fabs(segmento.a.y - no->p.y) <= EPS) ||
         (fabs(segmento.b.x - no->p.x) <= EPS &&
          fabs(segmento.b.y - no->p.y) <= EPS);
}

static bool validar_contra_segmentos(ptrNo raiz, ptrNo no_conexao,
                                     Segment novo_segmento,
                                     double distancia_minima) {
  Segment segmento_existente;

  if (raiz == NULL) {
    return true;
  }

  if (raiz->esq != NULL) {
    segmento_existente.a = raiz->p;
    segmento_existente.b = raiz->esq->p;

    if (!segmento_incidente_ao_no(segmento_existente, no_conexao)) {
      if (segmentos_intersectam(novo_segmento, segmento_existente)) {
        return false;
      }

      if (distancia_segmentos(novo_segmento, segmento_existente) <=
          distancia_minima) {
        return false;
      }
    }

    if (!validar_contra_segmentos(raiz->esq, no_conexao, novo_segmento,
                                  distancia_minima)) {
      return false;
    }
  }

  if (raiz->dir != NULL) {
    segmento_existente.a = raiz->p;
    segmento_existente.b = raiz->dir->p;

    if (!segmento_incidente_ao_no(segmento_existente, no_conexao)) {
      if (segmentos_intersectam(novo_segmento, segmento_existente)) {
        return false;
      }

      if (distancia_segmentos(novo_segmento, segmento_existente) <=
          distancia_minima) {
        return false;
      }
    }

    if (!validar_contra_segmentos(raiz->dir, no_conexao, novo_segmento,
                                  distancia_minima)) {
      return false;
    }
  }

  return true;
}

static bool conexao_valida(ptrNo raiz, ptrNo no_conexao, Point novo_ponto,
                           double raio, double distancia_minima) {
  Segment novo_segmento = {no_conexao->p, novo_ponto};

  if (!ponto_no_dominio(novo_ponto, raio)) {
    return false;
  }

  if (distancia_pontos(no_conexao->p, novo_ponto) <= distancia_minima) {
    return false;
  }

  return validar_contra_segmentos(raiz, no_conexao, novo_segmento,
                                  distancia_minima);
}

static void exportar_segmentos_rec(FILE *arquivo, ptrNo raiz) {
  if (raiz == NULL) {
    return;
  }

  if (raiz->esq != NULL) {
    fprintf(arquivo, "%d,%d,%.10f,%.10f,%.10f,%.10f\n", raiz->id, raiz->esq->id,
            raiz->p.x, raiz->p.y, raiz->esq->p.x, raiz->esq->p.y);
    exportar_segmentos_rec(arquivo, raiz->esq);
  }

  if (raiz->dir != NULL) {
    fprintf(arquivo, "%d,%d,%.10f,%.10f,%.10f,%.10f\n", raiz->id, raiz->dir->id,
            raiz->p.x, raiz->p.y, raiz->dir->p.x, raiz->dir->p.y);
    exportar_segmentos_rec(arquivo, raiz->dir);
  }
}

static bool exportar_segmentos_csv(const char *caminho, ptrNo raiz) {
  FILE *arquivo = fopen(caminho, "w");

  if (arquivo == NULL) {
    return false;
  }

  fprintf(arquivo, "parent_id,child_id,x1,y1,x2,y2\n");
  exportar_segmentos_rec(arquivo, raiz);
  fclose(arquivo);

  return true;
}

static void liberar_arvore(ptrNo raiz) {
  if (raiz == NULL) {
    return;
  }

  liberar_arvore(raiz->esq);
  liberar_arvore(raiz->dir);
  free(raiz);
}

static bool construir_arvore(ptrNo raiz, int nterm, double raio,
                             int *proximo_id, int *conexoes_rejeitadas) {
  int pontos_inseridos = 0;
  double distancia_minima = DISTANCE_EPSILON_FACTOR * raio;

  while (pontos_inseridos < nterm) {
    bool inseriu_ponto = false;

    for (int tentativa = 0; tentativa < MAX_POINT_ATTEMPTS && !inseriu_ponto;
         tentativa++) {
      Point novo_ponto = gerar_ponto_no_dominio(raio);
      int total_nos = contar_nos(raiz);
      Candidate *candidatos = malloc((size_t)total_nos * sizeof(Candidate));
      int total_candidatos = 0;

      if (candidatos == NULL) {
        return false;
      }

      preencher_candidatos(raiz, novo_ponto, candidatos, &total_candidatos);
      qsort(candidatos, (size_t)total_candidatos, sizeof(Candidate),
            comparar_candidatos);

      for (int i = 0; i < total_candidatos && !inseriu_ponto; i++) {
        ptrNo no_conexao = candidatos[i].no;

        if (conexao_valida(raiz, no_conexao, novo_ponto, raio,
                           distancia_minima)) {
          ptrNo novo_no = criar_no(*proximo_id, novo_ponto);

          if (novo_no == NULL) {
            free(candidatos);
            return false;
          }

          if (!inserir_no(no_conexao, novo_no)) {
            free(novo_no);
          } else {
            (*proximo_id)++;
            pontos_inseridos++;
            inseriu_ponto = true;
          }
        } else {
          (*conexoes_rejeitadas)++;
        }
      }

      free(candidatos);
    }

    if (!inseriu_ponto) {
      fprintf(stderr, "Nao foi possivel inserir o ponto terminal %d.\n",
              pontos_inseridos + 1);
      return false;
    }
  }

  return true;
}

static bool ler_inteiro_positivo(const char *texto, int *valor) {
  char *fim = NULL;
  long convertido = strtol(texto, &fim, 10);

  if (fim == texto || *fim != '\0' || convertido <= 0 ||
      convertido > 1000000L) {
    return false;
  }

  *valor = (int)convertido;
  return true;
}

static bool ler_double_positivo(const char *texto, double *valor) {
  char *fim = NULL;
  double convertido = strtod(texto, &fim);

  if (fim == texto || *fim != '\0' || convertido <= 0.0) {
    return false;
  }

  *valor = convertido;
  return true;
}

static void imprimir_uso(const char *programa) {
  fprintf(stderr, "Uso: %s Nterm R\n", programa);
}

int main(int argc, char **argv) {
  int nterm;
  int proximo_id = 1;
  int conexoes_rejeitadas = 0;
  double raio;
  Point ponto_raiz = {0.0, 0.0};
  ptrNo raiz;

  if (argc != 3) {
    imprimir_uso(argv[0]);
    return EXIT_FAILURE;
  }

  if (!ler_inteiro_positivo(argv[1], &nterm) ||
      !ler_double_positivo(argv[2], &raio)) {
    imprimir_uso(argv[0]);
    return EXIT_FAILURE;
  }

  srand((unsigned int)time(NULL));

  raiz = criar_no(0, ponto_raiz);
  if (raiz == NULL) {
    fprintf(stderr, "Erro ao alocar a raiz da arvore.\n");
    return EXIT_FAILURE;
  }

  if (!construir_arvore(raiz, nterm, raio, &proximo_id, &conexoes_rejeitadas)) {
    liberar_arvore(raiz);
    return EXIT_FAILURE;
  }

  if (!exportar_segmentos_csv(OUTPUT_FILE, raiz)) {
    fprintf(stderr, "Erro ao exportar o arquivo %s.\n", OUTPUT_FILE);
    liberar_arvore(raiz);
    return EXIT_FAILURE;
  }

  printf("Numero total de nos: %d\n", contar_nos(raiz));
  printf("Numero de folhas: %d\n", contar_folhas(raiz));
  printf("Comprimento total da arvore: %.6f\n", comprimento_total(raiz));
  printf("Numero de conexoes rejeitadas: %d\n", conexoes_rejeitadas);
  printf("Arquivo exportado: %s\n", OUTPUT_FILE);

  liberar_arvore(raiz);

  return EXIT_SUCCESS;
}
