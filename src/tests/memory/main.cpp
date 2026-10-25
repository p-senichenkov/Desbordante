#include "tests/common/all_csv_configs.h"
#include "tests/memory/algorithms.h"

int main() {
    memory::UCCPACVerifierMemory(tests::kIowa100k, {1, 2});
}
