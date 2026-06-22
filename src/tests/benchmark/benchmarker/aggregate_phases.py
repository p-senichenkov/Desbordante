# /// script
# requires-python = ">=3.11"
# dependencies = [
#     "click>=8.2.0, <9",
#     "matplotlib>=3.11.0, <4"
# ]
# ///

from pathlib import Path
import re
from dataclasses import dataclass

from matplotlib import pyplot as plt
import click

from data import Results

STAGE_RE = re.compile("(?P<stage_name>.+)")


@dataclass
class StageResult:
    stage_name: str
    mean: float
    half_conf_int: float


def read_file(fname: Path) -> list[StageResult]:
    with open(fname, "r") as f:
        results = Results.load(f.read())
    return [StageResult(res.algo_name, res.mean, res.conf_int_half) for res in results]


def read_files(fnames: dict[str, Path]) -> dict[str, list[StageResult]]:
    result = {}
    for algo_name, fname in fnames.items():
        print(algo_name)
        result[algo_name] = read_file(fname)
        print(
            "\n".join(
                [
                    f"\t{stage_res.stage_name}: {stage_res.mean}±{stage_res.half_conf_int}ms"
                    for stage_res in result[algo_name]
                ]
            )
        )
    return result


PHASE2LABEL = [
    ("Load table", "Чтение таблицы"),
    ("Prepare types and tuples", "Объединение значений в кортежи"),
    ("Calculate distances", "Вычисление расстояний"),
    ("Sort distances", "Сортировка расстояний"),
    ("PAC-Man", "PAC-Man"),
    ("Calculate highlights", "Вычисление исключений"),
]


def build_plots(results: dict[str, list[StageResult]]) -> None:
    # Pyplot expects another format: {"algo": [time]}
    # TODO: Looks like it expects {"stage": [times]}. Need to experiment a bit
    stage_names = [phase2label[1] for phase2label in PHASE2LABEL]

    # TODO: Display confidence interval?
    means: dict[str, list[float]] = {}
    for algo_name, algo_results in results.items():
        algo_means = []
        for phase2label in PHASE2LABEL:
            for res in algo_results:
                if res.stage_name == phase2label[0]:
                    algo_means.append(res.mean)
                    break
            else:
                assert False, f"No {phase2label[0]} for {algo_name}"
            means[algo_name] = algo_means

    plt.yscale("log")
    plt.grouped_bar(means, tick_labels=stage_names)
    plt.legend()
    plt.show()


@click.command()
@click.option("-f", "--filenames", multiple=True, type=str)
@click.option("-a", "--algo-names", multiple=True, type=str)
def main(filenames: list[str], algo_names: list[str]):
    results = read_files(
        {
            algo_name: Path(filename)
            for algo_name, filename in zip(algo_names, filenames)
        }
    )
    build_plots(results)


if __name__ == "__main__":
    main()
