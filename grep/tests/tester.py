import subprocess
import json
from pathlib import Path


def red(text)   : return f'\033[1;31m{text}\033[0m'
def green(text) : return f'\033[1;32m{text}\033[0m'
def yellow(text): return f'\033[1;33m{text}\033[0m'
def blue(text)  : return f'\033[1;34m{text}\033[0m'


def run_test(test):
    name   = test['name']
    files  = test['files']
    idol   = test['idol']
    mimic  = test['mimic']
    args   = test['args']

    if not hasattr(run_test, 'test_no'):
        run_test.test_no = 0
    run_test.test_no += 1

    with open('idol-output', 'w') as ostream:
        subprocess.run(
            [idol, *args, *files],
            stdout=ostream,
            text=True
        )

    with open('mimic-output', 'w') as ostream:
        subprocess.run(
            [mimic, *args, *files],
            stdout=ostream,
            text=True
        )

    check_test = subprocess.run(
        ["cmp", "-s", "idol-output", "mimic-output"],
        # stdout=subprocess.DEVNULL, # suppress process output
        # stderr=subprocess.DEVNULL,
    )

    if (check_test.returncode == 0):
        print("[", green("PASSED"), "]", end=" ")
    else:
        print("[", red("FAILED"), "]", end=" ")
    print(f"{run_test.test_no}/{ALL_TESTS}", end=" ")
    print(name, end=" ")
    print(args, sep="", end=" ")
    print("(", blue(files), ")", sep="")

    if (check_test.returncode == 0):
        return 1
    else:
        return 0


# Load file with tests
with open('tests.json', 'r', encoding='utf-8') as json_tests:
    tests = json.load(json_tests)


# Run all tests
ALL_TESTS = len(tests)
success = 0
for test in tests:
    success += run_test(test)

# Tests review
print(f"Result {success}/{ALL_TESTS}")


# Clean up
idol_tmp = Path('idol-output')
mimic_tmp = Path('mimic-output')

if idol_tmp.exists():
    idol_tmp.unlink()
if mimic_tmp.exists():
    mimic_tmp.unlink()

