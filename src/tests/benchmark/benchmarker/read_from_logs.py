"""Run benchmarks and read results from logs"""

import re
import subprocess
import logging
from collections import defaultdict

LOG_PATTERN = re.compile("(?P<name>.+) took (?P<time>[0-9]+)ms")


def extract_times(stdout: list[str]) -> dict[str, float]:
    result = {}
    stage_num = 0
    for line in stdout:
        m = LOG_PATTERN.fullmatch(line)
        if m:
            name = m.group("name")
            time = float(m.group("time"))
            if name in result:
                logging.warning(f"Duplicate phase: {name}")
            result[f"{stage_num}. {name}"] = time
            stage_num += 1
    return result


def single_run(executable: str) -> dict[str, float]:
    proc = subprocess.run(executable, capture_output=True, text=True)
    print(proc.stdout)
    print()
    stdout = proc.stdout.splitlines()
    return extract_times(stdout)


def run_all(executable: str, runs: int) -> dict[str, list[float]]:
    result = defaultdict(list)
    for _ in range(runs):
        times = single_run(executable)
        for name, time in times.items():
            result[name].append(time)
    return result
