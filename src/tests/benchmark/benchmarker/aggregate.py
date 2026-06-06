from collections import namedtuple
import pickle
from pathlib import Path
import re

import click

from data import Results

AggregatedData = namedtuple('AggregatedData', ['x', 'y', 'yerror'])


FD_PAC_IOWA_RE = re.compile('.+PACVerifier, iowa([0-9]+)k')


def read_file(fname: Path) -> list[AggregatedData]:
    result = []
    with open(fname, 'r') as f:
        results = Results.load(f.read())
    for res in results:
        m = FD_PAC_IOWA_RE.match(res.algo_name)
        if not m:
            raise ValueError('Bad algo name')
        num_rows = int(m.group(1)) * 1000
        mean = res.mean
        error = res.conf_int_half
        result.append(AggregatedData(num_rows, mean, error))
    return result


@click.command()
@click.option('-f', '--fnames', type=str, multiple=True)
@click.option('-o', '--output', type=str)
def main(fnames: list[str], output: str):
    aggregated = []
    for fname in fnames:
        aggregated.append(read_file(Path(fname)))
    print(aggregated)
    with open(output, 'wb') as f:
        pickle.dump(aggregated, f)

if __name__ == '__main__':
    main()
