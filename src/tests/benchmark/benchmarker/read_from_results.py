"""Run benchmarks and read results from JSONs"""

import typing as tp
from datetime import timedelta, date
from pathlib import Path
import subprocess
from collections import defaultdict

from pydantic import BaseModel, BeforeValidator, PlainSerializer

MillisTimeDelta = tp.Annotated[
    timedelta,
    BeforeValidator(lambda v: timedelta(milliseconds=v)),
    PlainSerializer(lambda td: td / timedelta(milliseconds=1), return_type=int),
]


class Result(BaseModel):
    name: str
    time: MillisTimeDelta


class Results(BaseModel):
    date: date
    results: list[Result]


# Read serialized results from JSON file
def read_results(filename: Path) -> Results:
    with open(filename, "r") as file:
        return Results.model_validate_json(file.read())


def run_all(executable: str, runs: int, results_dir: Path) -> dict[str, list[float]]:
    result: dict[str, list[float]] = defaultdict(list)

    for i in range(runs):
        out_fname = results_dir / f"{i}.json"
        subprocess.run([executable, "--output", out_fname])
        results = read_results(out_fname)
        for res in results.results:
            result[res.name].append(res.time / timedelta(milliseconds=1))

    return result
