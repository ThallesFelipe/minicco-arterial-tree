import argparse
import csv

import numpy as np
import pyvista as pv


def carregar_segmentos(caminho):
    pontos_por_id = {}
    segmentos = []

    with open(caminho, newline="", encoding="utf-8") as arquivo:
        leitor = csv.DictReader(arquivo)

        for linha in leitor:
            id_pai = int(linha["parent_id"])
            id_filho = int(linha["child_id"])

            pontos_por_id[id_pai] = (
                float(linha["x1"]),
                float(linha["y1"]),
                0.0,
            )
            pontos_por_id[id_filho] = (
                float(linha["x2"]),
                float(linha["y2"]),
                0.0,
            )
            segmentos.append((id_pai, id_filho))

    ids_ordenados = sorted(pontos_por_id)
    indice_por_id = {id_no: indice for indice, id_no in enumerate(ids_ordenados)}
    pontos = np.array([pontos_por_id[id_no] for id_no in ids_ordenados])
    linhas = []

    for id_pai, id_filho in segmentos:
        linhas.extend([2, indice_por_id[id_pai], indice_por_id[id_filho]])

    return pontos, np.array(linhas)


def visualizar_arvore(caminho):
    pontos, linhas = carregar_segmentos(caminho)

    mesh = pv.PolyData()
    mesh.points = pontos
    mesh.lines = linhas

    plotter = pv.Plotter()
    plotter.add_mesh(mesh, color="red", line_width=3)
    plotter.add_points(pontos, color="black", point_size=8, render_points_as_spheres=True)
    plotter.show()


def main():
    parser = argparse.ArgumentParser(description="Visualiza a arvore arterial gerada pelo MiniCCO-0.")
    parser.add_argument("arquivo", nargs="?", default="segments.csv", help="Arquivo CSV exportado pelo programa em C.")
    args = parser.parse_args()

    visualizar_arvore(args.arquivo)


if __name__ == "__main__":
    main()
