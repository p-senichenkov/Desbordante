from collections import namedtuple, defaultdict
from pathlib import Path
import re
from dataclasses import dataclass
from matplotlib import pyplot as plt

import click

from data import Results

AggregatedData = namedtuple("AggregatedData", ["x", "y", "yerror", "algo_name"])


DOMAIN_PAC_IOWA_RE = re.compile("DomainPACVerifier, iowa([0-9]+)k")
FD_PAC_IOWA_RE = re.compile("FDPACVerifier, iowa([0-9]+)k")
UCC_PAC_IOWA_RE = re.compile("UCCPACVerifier, iowa([0-9]+)k")

name2re = {
    "Domain PAC Verifier": DOMAIN_PAC_IOWA_RE,
    "FD PAC Verifier": FD_PAC_IOWA_RE,
    "UCC PAC Verifier": UCC_PAC_IOWA_RE,
}


def read_file(fname: Path) -> list[AggregatedData]:
    result: list[AggregatedData] = []
    with open(fname, "r") as f:
        results = Results.load(f.read())
    for res in results:
        for name, regexp in name2re.items():
            m = regexp.match(res.algo_name)
            if m:
                num_rows = int(m.group(1)) * 1000
                mean = res.mean
                error = res.conf_int_half
                result.append(AggregatedData(num_rows, mean, error, name))
                break
        else:
            raise ValueError(f"Algo name {res.algo_name} doesn't match any of regexps")
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
    plt.xlabel("Количество строк в таблице, тыс.")
    plt.ylabel("Время работы, сек.")
    for name, aggregated in aggregated_by_name.items():
        plt.errorbar(
            [v / 1000 for v in aggregated.xs],
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
