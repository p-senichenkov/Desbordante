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
import time
import shutil

import click
from scipy import stats
import numpy

import read_from_results
import read_from_logs
from data import Results


def process_results(results: dict[str, list[float]]) -> list[Results]:
    result: list[Results] = []
    for algo_name, raw_results in results.items():
        normaltest_p_value = float(stats.normaltest(raw_results).pvalue)
        shapiro_p_value = float(stats.shapiro(raw_results).pvalue)
        dropped_by_iqr = False
        if normaltest_p_value < 0.05 and shapiro_p_value < 0.05:
            print(f"WARNING: None of normal tests passed for {algo_name}")
            print("Trying to drop outlier")
            # Use interquantile range to determine if outlier can be dropped
            raw_results.sort()
            q25 = numpy.percentile(raw_results, 25)
            q75 = numpy.percentile(raw_results, 75)
            iqr = q75 - q25
            lower = q25 - 1.5 * iqr
            upper = q75 - 1.5 * iqr
            if raw_results[-1] > upper:
                dropped_by_iqr = raw_results[-1]
                del raw_results[-1]
            elif raw_results[0] < lower:
                dropped_by_iqr = raw_results[0]
                del raw_results[0]
            normaltest_p_value = float(stats.normaltest(raw_results).pvalue)
            shapiro_p_value = float(stats.shapiro(raw_results).pvalue)
            if normaltest_p_value < 0.05 and shapiro_p_value < 0.05:
                print("!!! Still don't pass !!!")
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
                dropped_by_iqr=dropped_by_iqr,
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
    shutil.rmtree(results_dir, ignore_errors=True)
    results_dir.mkdir(parents=True)

    results = read_from_results.run_all(executable, num_runs, results_dir)
    cooked_results = process_results(results)
    with open(results_dir / "all_results.json", "w") as f:
        f.write(Results.pretty_dump(cooked_results))


@cli.command()
@click.option("-e", "--executable", default="./Desbordante.benchmark")
@click.option("-n", "--num-runs", default=20)
@click.option("-d", "--res-dir", default=time.ctime())
def logs(executable: str, num_runs: int, res_dir):
    results_dir = Path("results") / res_dir
    shutil.rmtree(results_dir, ignore_errors=True)
    results_dir.mkdir(parents=True)

    results = read_from_logs.run_all(executable, num_runs)
    cooked_results = process_results(results)
    with open(results_dir / "all_results.json", "w") as f:
        f.write(Results.pretty_dump(cooked_results))


if __name__ == "__main__":
    cli()
