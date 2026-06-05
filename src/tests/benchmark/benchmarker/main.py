# /// script
# requires-python = ">=3.10"
# dependencies = [
#     "click>=8.2.0, <9",
#     "matplotlib>=3.8.0, <4",
#     "pydantic>=2.10.0, <3",
#     "scipy"
# ]
# ///

from pathlib import Path
from dataclasses import dataclass, asdict
import json
import time
import shutil

import click
from scipy import stats
import numpy

import read_from_results


@dataclass(frozen=True)
class Results:
    algo_name: str
    raw_results: list[float]
    normaltest_p_value: float
    shapiro_p_value: float
    mean: float
    std: float
    conf_int_half: float

    @staticmethod
    def pretty_dump(results) -> str:
        lst = [asdict(res) for res in results]
        return json.dumps(lst, indent=2, sort_keys=True)

    @staticmethod
    def load(s: str):
        lst = json.loads(s)
        return [Results(**dct) for dct in lst]


def process_results(results: dict[str, list[float]]) -> list[Results]:
    result: list[Results] = []
    for algo_name, raw_results in results.items():
        normaltest_p_value = float(stats.normaltest(raw_results).pvalue)
        shapiro_p_value = float(stats.shapiro(raw_results).pvalue)
        if normaltest_p_value < 0.05 and shapiro_p_value < 0.05:
            print(f"WARNING: None of normal tests passed for {algo_name}")
        mean = float(numpy.mean(raw_results))
        std = float(numpy.std(raw_results, ddof=1))
        conf_int_half = stats.t.ppf(0.975, df=len(raw_results) - 1) * stats.sem(
            raw_results
        )
        result.append(
            Results(
                algo_name=algo_name,
                raw_results=raw_results,
                normaltest_p_value=normaltest_p_value,
                shapiro_p_value=shapiro_p_value,
                mean=mean,
                std=std,
                conf_int_half=conf_int_half,
            )
        )
    return result


@click.group()
def cli():
    pass


@cli.command()
@click.option("-e", "--executable", default="./Desbordante.benchmark")
@click.option("-n", "--num-runs", default=20)
@click.option("-d", "--res-dir", default=time.ctime())
def results(executable: str, num_runs: int, res_dir: str):
    results_dir = Path("results") / res_dir
    if results_dir.exists():
        shutil.rmtree(results_dir)
    results_dir.mkdir(parents=True)

    results = read_from_results.run_all(executable, num_runs, results_dir)
    cooked_results = process_results(results)
    with open(results_dir / "all_results.json", "w") as f:
        f.write(Results.pretty_dump(cooked_results))


if __name__ == "__main__":
    cli()
