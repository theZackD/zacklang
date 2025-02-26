# Add benchmarks
add_executable(test_dead_block_bench
    tests/zir/benchmarks/test_dead_block_bench.cpp)
target_link_libraries(test_dead_block_bench PRIVATE zir)

# Add block merging test
add_executable(test_block_merging
    tests/zir/test_block_merging.cpp)
target_link_libraries(test_block_merging PRIVATE zir) 