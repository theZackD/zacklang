# Makefile additions
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I include

# Add ZIR objects
ZIR_SRCS = src/zir_c_api.cpp
ZIR_SRCS += src/zir_value.cpp
ZIR_SRCS += src/zir_basic_block.cpp
ZIR_SRCS += src/zir_function.cpp
ZIR_OBJS = $(ZIR_SRCS:.cpp=.o)

# Add ZIR test
test_zir_basic: tests/zir/test_zir_basic.cpp $(ZIR_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@
	./$@

# Add new test target
test_zir_safety: tests/zir/test_zir_safety.cpp $(ZIR_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@
	./$@

# Add memory test target with AddressSanitizer
.PHONY: test_zir_memory
test_zir_memory: tests/zir/memory/test_zir_memory.cpp $(ZIR_OBJS)
	$(CXX) $(CXXFLAGS) -fsanitize=address $^ -o $@
	./$@
	rm -f $@

# Add value test target
.PHONY: test_zir_value
test_zir_value: tests/zir/test_zir_value.cpp $(ZIR_OBJS)
	$(CXX) $(CXXFLAGS) -fsanitize=address $^ -o $@
	./$@
	rm -f $@

# Add integer test target
.PHONY: test_zir_integer
test_zir_integer: tests/zir/test_zir_integer.cpp $(ZIR_OBJS)
	$(CXX) $(CXXFLAGS) -fsanitize=address $^ -o $@
	./$@
	rm -f $@

# Add float test target
.PHONY: test_zir_float
test_zir_float: tests/zir/test_zir_float.cpp $(ZIR_OBJS)
	$(CXX) $(CXXFLAGS) -fsanitize=address $^ -o $@
	./$@
	rm -f $@

# Add boolean test target
.PHONY: test_zir_boolean
test_zir_boolean: tests/zir/test_zir_boolean.cpp $(ZIR_OBJS)
	$(CXX) $(CXXFLAGS) -fsanitize=address $^ -o $@
	./$@
	rm -f $@

# Add dead block analysis test target
.PHONY: test_zir_dead_blocks
test_zir_dead_blocks: tests/zir/test_zir_dead_blocks.cpp $(ZIR_OBJS)
	$(CXX) $(CXXFLAGS) -fsanitize=address $^ -o $@
	./$@
	rm -f $@

# Add string test target
.PHONY: test_zir_string
test_zir_string: tests/zir/test_zir_string.cpp $(ZIR_OBJS)
	$(CXX) $(CXXFLAGS) -fsanitize=address $^ -o $@
	./$@
	rm -f $@

# Add C API test target
.PHONY: test_zir_c_api
test_zir_c_api: tests/zir/test_zir_c_api.cpp $(ZIR_OBJS)
	$(CXX) $(CXXFLAGS) -fsanitize=address $^ -o $@
	./$@
	rm -f $@

# Add basic block test target
.PHONY: test_zir_basic_block
test_zir_basic_block: tests/zir/test_zir_basic_block.cpp $(ZIR_OBJS)
	$(CXX) $(CXXFLAGS) -fsanitize=address $^ -o $@
	./$@
	rm -f $@

# Add function test target
.PHONY: test_zir_function
test_zir_function: tests/zir/test_zir_function.cpp $(ZIR_OBJS)
	$(CXX) $(CXXFLAGS) -fsanitize=address $^ -o $@
	./$@
	rm -f $@

# Add instruction test target
.PHONY: test_zir_instruction
test_zir_instruction: tests/zir/test_zir_instruction.cpp $(ZIR_OBJS)
	$(CXX) $(CXXFLAGS) -fsanitize=address $^ -o $@
	./$@
	rm -f $@

# Add arithmetic instruction test target
.PHONY: test_zir_arithmetic
test_zir_arithmetic: tests/zir/test_zir_arithmetic.cpp $(ZIR_OBJS)
	$(CXX) $(CXXFLAGS) -fsanitize=address $^ -o $@
	./$@
	rm -f $@

# Add comparison instruction test target
.PHONY: test_zir_comparison
test_zir_comparison: tests/zir/test_zir_comparison.cpp $(ZIR_OBJS)
	$(CXX) $(CXXFLAGS) -fsanitize=address $^ -o $@
	./$@
	rm -f $@

# Add logical instruction test target
.PHONY: test_zir_logical
test_zir_logical: tests/zir/test_zir_logical.cpp $(ZIR_OBJS)
	$(CXX) $(CXXFLAGS) -fsanitize=address $^ -o $@
	./$@
	rm -f $@

# Add absolute value example test target
.PHONY: test_zir_abs_example
test_zir_abs_example: tests/zir/examples/test_zir_abs.cpp $(ZIR_OBJS)
	$(CXX) $(CXXFLAGS) -fsanitize=address $^ -o $@
	./$@
	rm -f $@

# Add control flow instruction test target
.PHONY: test_zir_control_flow
test_zir_control_flow: tests/zir/test_zir_control_flow.cpp $(ZIR_OBJS)
	$(CXX) $(CXXFLAGS) -fsanitize=address $^ -o $@
	./$@
	rm -f $@

# Add block links test target
.PHONY: test_zir_block_links
test_zir_block_links: tests/zir/test_zir_block_links.cpp $(ZIR_OBJS)
	$(CXX) $(CXXFLAGS) -fsanitize=address $^ -o $@
	./$@
	rm -f $@

# Add graph analysis test target
.PHONY: test_zir_graph_analysis
test_zir_graph_analysis: tests/zir/test_zir_graph_analysis.cpp $(ZIR_OBJS)
	$(CXX) $(CXXFLAGS) -fsanitize=address $^ -o $@
	./$@
	rm -f $@

# Add dead block elimination benchmark target
.PHONY: test_dead_block_bench
test_dead_block_bench: tests/zir/benchmarks/test_dead_block_bench.cpp $(ZIR_OBJS)
	$(CXX) $(CXXFLAGS) -O3 $^ -o $@
	./$@
	rm -f $@

# Update test target
test: test_zir_basic test_zir_safety test_zir_memory test_zir_value test_zir_integer test_zir_float test_zir_boolean test_zir_string test_zir_c_api test_zir_basic_block test_zir_function test_zir_instruction test_zir_arithmetic test_zir_comparison test_zir_logical test_zir_abs_example test_zir_control_flow test_zir_block_links test_zir_graph_analysis test_zir_dead_blocks test_dead_block_bench