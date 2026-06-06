from dataclasses import dataclass
import json


@dataclass(frozen=True)
class Results:
    algo_name: str
    raw_results: list[float]
    normaltest_p_value: float
    shapiro_p_value: float
    mean: float
    std: float
    conf_int_half: float
    dropped_by_iqr: bool | float

    @staticmethod
    def pretty_dump(results) -> str:
        lst = [asdict(res) for res in results]
        return json.dumps(lst, indent=2, sort_keys=True)

    @staticmethod
    def load(s: str):
        lst = json.loads(s)
        return [Results(**dct) for dct in lst]
