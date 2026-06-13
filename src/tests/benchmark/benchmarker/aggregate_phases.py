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

STAGE_RE = re.compile("(?P<stage_num>[0-9]+)[.] (?P<stage_name>.+)")


@dataclass
class StageResult:
    stage_name: str
    mean: float
    half_conf_int: float


def read_file(fname: Path) -> list[StageResult]:
    result: dict[int, StageResult] = {}
    with open(fname, "r") as f:
        results = Results.load(f.read())
    for res in results:
        m = STAGE_RE.fullmatch(res.algo_name)
        assert m is not None
        stage_num = int(m.group("stage_num"))
        stage_name = m.group("stage_name")
        result[stage_num] = StageResult(stage_name, res.mean, res.conf_int_half)
    return [result[key] for key in sorted(result.keys())]


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


def build_plots(results: dict[str, list[StageResult]]) -> None:
    # Pyplot expects another format: {"algo": [time]}
    # TODO: Looks like it expects {"stage": [times]}. Need to experiment a bit
    stage_names = [stage_res.stage_name for stage_res in list(results.values())[0]]
    algo_results = {
        algo_name: [stage_res.mean for stage_res in stage_results]
        for algo_name, stage_results in results.items()
    }

    plt.yscale("log")
    plt.grouped_bar(algo_results, tick_labels=stage_names)
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


if __name__ == '__main__':
    main()
