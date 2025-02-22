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

# Update test target
test: test_zir_basic test_zir_safety test_zir_memory test_zir_value test_zir_integer test_zir_float test_zir_boolean test_zir_string test_zir_c_api test_zir_basic_block test_zir_function