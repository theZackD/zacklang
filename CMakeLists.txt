# Add benchmarks
add_executable(test_dead_block_bench
    tests/zir/benchmarks/test_dead_block_bench.cpp)
target_link_libraries(test_dead_block_bench PRIVATE zir) 