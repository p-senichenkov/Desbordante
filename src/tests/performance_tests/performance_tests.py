import json
from sys import argv

THRESHOLD = 10 # in percent

# Different thresholds for some specific algos
DYNAMIC_THRESHOLDS = {'Pyro': 15}

# Process JSON file with results, produced by googletest
def process_results(filename: str) -> dict[str: int]:
    results = dict()

    with open(filename, 'r') as file:
        result = json.load(file)

    # Iterate through testsuites:
    testsuites = result['testsuites']
    for testsuite in testsuites:
        # Iterate through tests:
        tests = testsuite['testsuite']
        for test in tests:
            # Slashes are used in paramethrized tests
            name = test['name'].replace('/', '_')
            raw_time = test['time']
            # Time is a string of form ***s. Convert it to ms:
            time_in_seconds = raw_time[:-1]
            time = int(float(time_in_seconds) * 1000)
            results[name] = time

    return results

# Save results to JSON and save to file
def serialize_results(results: dict[str: int], filename: str) -> None:
    with open(filename, 'w') as file:
        json.dump(results, file)

# Read serialized results from JSON file
def read_results(filename: str) -> dict[str: int]:
    with open(filename, 'r') as file:
        return json.load(file)

# Check one test
def test(name: str, old_results: dict[str: int], curr_result: dict[str: int]) -> bool:
    if name in old_results:
        old_res = old_results[name]
        curr_res = curr_result[name]

        diff = curr_res - old_res
        if old_res > 0:
            overhead = diff / old_res * 100
        else:
            # All performance tests should work a long time, so it's impossible situation
            # But for debugging purposes, take THRESHOLD milliseconds as normal overhead from zero
            overhead = diff / THRESHOLD * 100

        threshold = THRESHOLD
        for dyn_algo_name in DYNAMIC_THRESHOLDS:
            if name.startswith(dyn_algo_name):
                threshold = DYNAMIC_THRESHOLDS[dyn_algo_name]
                break

        if overhead <= threshold:
            print(f'{name}: SUCESS: difference is {overhead}%')
            return True

        print(f'{name}: FAIL: difference is {overhead}%')
        return False

    print(f'{name}: WARNING: no such test in previous run')
    return True # It's warning, not an error

def test_all(old_results: dict[str: int], curr_results: dict[str: int]) -> bool:
    success = True
    for test_name in curr_results:
        success &= test(test_name, old_results, curr_results)
    return success

# First argument is googletest result file
# Secong argument is file to save results
# Third argument is file with previous results
# Exit codes:
#   0 -- success
#   1 -- invalid call
#   2 -- test failure
def main():
    if len(argv) >= 4:
        results = process_results(argv[1])
        old_results = read_results(argv[3])

        serialize_results(results, argv[2])
        success = test_all(old_results, results)
        exit(0 if success else 2)
    else:
        print('Please, specify all needed filenames')
        exit(1)

if __name__ == '__main__':
    main()
