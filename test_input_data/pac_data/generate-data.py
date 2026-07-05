import csv
from pathlib import Path

import numpy as np

COLUMN_COUNT = 60
LINE_COUNT = 50_000

rng = np.random.default_rng()

headers = list(range(COLUMN_COUNT))
columns = rng.uniform(1, 100, (COLUMN_COUNT, LINE_COUNT))
rows = np.transpose(columns)

OUTPUT = Path("uniform-wide.csv")
with open(OUTPUT, "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(headers)
    writer.writerows(rows)
