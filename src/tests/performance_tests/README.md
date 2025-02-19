# Performance tests

If you are adding performance tests, please read ths instruction

## Common principles

Each performance test should execute *only one* algorithm on some large dataset.
It should end with `SUCEED()` macro call.

For more examples see `HyFD` and `AFDTane` tests.

## Execution time

Good execution time for a performance test is about 5 minutes.
Note that threshold for execution time is 10%.
If your algorithm run time can fluctuate greatly, consider using a larger dataset.
TODO(senichenkov): add possibility to select threshold for each algorithm

## Naming conventions

Test suite name **must** contain `HeavyDatasets` to prevent these tests from running by default
and `PerformanceTests` to enable their execution in appropriate workflow.

### Regular tests

For regular tests script selects only test name, so make sure to give your regular tests meaningful
names.
Also, test name should contain dataset name so that we can have right statistics.

For example, if you use `TEST` macro, test for `YourAlgo` on `Iris.csv` dataset should start
like this:
```C++
TEST(HugeDatasetsPerformanceTests, YourAlgo_Iris) {
```

For more examples see `HyFD` test.

### Value-parametrized tests

For value-parametrized tests script selects test name and paramether string representation.
Make sure to define test suffix generator (see [Specifying Names for Value-Parameterized Test Parameters](http://google.github.io/googletest/advanced.html#specifying-names-for-value-parameterized-test-parameters)).
Note that such generators can return **only ASCII characters** (even an underscore is forbidden).

For more examples see `AFDTane` tests.

### Type-parametrized tests

TODO(senichenkov): type-parametrized tests
