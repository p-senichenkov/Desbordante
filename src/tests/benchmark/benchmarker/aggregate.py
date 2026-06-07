from collections import namedtuple
from pathlib import Path
import re
from dataclasses import dataclass
from matplotlib import pyplot as plt

import click

from data import Results

AggregatedData = namedtuple("AggregatedData", ["x", "y", "yerror"])


FD_PAC_IOWA_RE = re.compile(".+PACVerifier, iowa([0-9]+)k")


def read_file(fname: Path) -> list[AggregatedData]:
    result = []
    with open(fname, "r") as f:
        results = Results.load(f.read())
    for res in results:
        m = FD_PAC_IOWA_RE.match(res.algo_name)
        if not m:
            raise ValueError("Bad algo name")
        num_rows = int(m.group(1)) * 1000
        mean = res.mean
        error = res.conf_int_half
        result.append(AggregatedData(num_rows, mean, error))
    return result


@dataclass
class AggregatedDataList:
    xs: list[float]
    ys: list[float]
    yerrs: list[float]


def read_files(fnames: list[Path]) -> AggregatedDataList:
    aggregated_results: list[AggregatedData] = []
    for fname in fnames:
        aggregated = read_file(fname)
        aggregated_results.extend(aggregated)
    aggregated_results.sort(key=lambda aggr: aggr.x)

    xs, ys, yerrs = [], [], []
    for aggr_data in aggregated_results:
        if aggr_data.x in xs:
            print(f"WARNING: {aggr_data.x} duplicate")
        xs.append(aggr_data.x)
        ys.append(aggr_data.y)
        yerrs.append(aggr_data.yerror)
    return AggregatedDataList(xs, ys, yerrs)


def build_plot(aggregated: AggregatedDataList) -> None:
    plt.xscale("log")
    plt.yscale("log")
    plt.errorbar(aggregated.xs, aggregated.ys, aggregated.yerrs)
    plt.show()


@click.command()
@click.option("-f", "--fnames", type=str, multiple=True)
def main(fnames: list[str]):
    aggregated = read_files([Path(fname) for fname in fnames])
    print(aggregated)
    build_plot(aggregated)


if __name__ == "__main__":
    main()
