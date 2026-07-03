# /// script
# requires-python = ">=3.10"
# dependencies = [
#     "click>=8.2.0, <9",
#     "matplotlib>=3.8.0, <4",
#     "PyQt6>=6.0",
# ]
# ///
from collections import namedtuple, defaultdict
from pathlib import Path
import re
from dataclasses import dataclass
from matplotlib import pyplot as plt

import click

from data import Results

AggregatedData = namedtuple("AggregatedData", ["x", "y", "yerror", "algo_name"])


ALGO_ATTR_RE = re.compile("(?P<algo_name>[a-zA-Z]+), uniform-wide, (?P<attributes>[0-9]+) attributes")


def read_file(fname: Path) -> list[AggregatedData]:
    result: list[AggregatedData] = []
    with open(fname, "r") as f:
        results = Results.load(f.read())
    for res in results:
        print(res.algo_name)
        m = ALGO_ATTR_RE.match(res.algo_name)
        assert m is not None
        result.append(AggregatedData(int(m.group("attributes")), res.mean, res.conf_int_half,
                                     m.group("algo_name")))
    return result


@dataclass
class AggregatedDataList:
    xs: list[float]
    ys: list[float]
    yerrs: list[float]


def read_files(fnames: list[Path]) -> dict[str, AggregatedDataList]:
    results_by_name: dict[str, list[AggregatedData]] = defaultdict(list)
    for fname in fnames:
        aggregated = read_file(fname)
        for aggr in aggregated:
            results_by_name[aggr.algo_name].append(aggr)
    for results in results_by_name.values():
        results.sort(key=lambda aggr: aggr.x)

    result: dict[str, AggregatedDataList] = {}
    for name, aggregated_results in results_by_name.items():
        xs, ys, yerrs = [], [], []
        for aggr_data in aggregated_results:
            if aggr_data.x in xs:
                print(f"WARNING: {aggr_data.x} duplicate")
            xs.append(aggr_data.x)
            ys.append(aggr_data.y)
            yerrs.append(aggr_data.yerror)
        result[name] = AggregatedDataList(xs, ys, yerrs)
    return result


def build_plot(aggregated_by_name: dict[str, AggregatedDataList]) -> None:
    # plt.xscale("log")
    # plt.yscale("log")
    plt.xlabel("Арность зависимости")
    plt.ylabel("Время работы, сек.")
    for name, aggregated in aggregated_by_name.items():
        plt.errorbar(
            aggregated.xs,
            [v / 1000 for v in aggregated.ys],
            [v / 1000 for v in aggregated.yerrs],
            label=name,
        )
    plt.legend()
    plt.show()


@click.command()
@click.option("-f", "--fnames", type=str, multiple=True)
def main(fnames: list[str]):
    aggregated = read_files([Path(fname) for fname in fnames])
    print(aggregated)
    build_plot(aggregated)


if __name__ == "__main__":
    main()
