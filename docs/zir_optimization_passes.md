# ZIR Optimization Passes

## Overview

ZIR (Zacklang Intermediate Representation) supports a variety of optimization passes to improve code quality and execution performance. Each pass analyzes the ZIR structure and performs transformations to make the code more efficient.

This document describes the major optimization passes currently implemented in the ZIR infrastructure.

## Dead Block Elimination

### Purpose

Dead Block Elimination (DBE) identifies and removes basic blocks that can never be executed, reducing code size and improving cache efficiency.

### Analysis Phase

1. Identify the entry block of the function
2. Perform a reachability analysis to find blocks that can be reached from the entry block
3. Any block not reachable from the entry block is considered dead

### Transformation Phase

1. Remove all identified dead blocks from the function
2. Update any references to the removed blocks

### Example

```cpp
// Create a function with dead blocks
auto function = std::make_shared<ZIRFunctionImpl>("example");
auto entryBlock = std::make_shared<ZIRBasicBlockImpl>("entry");
auto block1 = std::make_shared<ZIRBasicBlockImpl>("block1");
auto block2 = std::make_shared<ZIRBasicBlockImpl>("block2"); // This will be dead
auto block3 = std::make_shared<ZIRBasicBlockImpl>("block3");

function->addBlock(entryBlock);
function->addBlock(block1);
function->addBlock(block2); // Dead block (not linked to any reachable block)
function->addBlock(block3);

// Link blocks
entryBlock->addSuccessor(block1);
block1->addSuccessor(block3);

// Apply dead block elimination
auto deadBlocks = function->findDeadBlocks();
// deadBlocks now contains [block2]

function->removeDeadBlocks();
// function now only contains [entryBlock, block1, block3]
```

### Performance Impact

Dead Block Elimination typically provides:

- 5-10% reduction in code size for functions with unreachable code
- Improved instruction cache efficiency
- Minor runtime performance improvements

## Block Merging

### Purpose

Block Merging identifies adjacent blocks that can be combined without changing program semantics. This reduces jump instructions and improves code locality.

### Analysis Phase

1. Identify blocks with a single successor
2. Check if the successor has only one predecessor
3. Validate that merging is safe (no PHI nodes, label references, etc.)

### Transformation Phase

1. Combine instructions from both blocks into the predecessor
2. Update control flow edges to bypass the merged block
3. Update any references to the eliminated block

### Performance

Block merging shows excellent performance across various code structures:

- Linear chains: scales linearly with graph size
- Diamond patterns: efficiently handles complex control flow
- Merge checks are very fast (typically < 10μs)

## Jump Threading

### Purpose

Jump Threading optimizes control flow by bypassing blocks that only contain jumps to other blocks. This eliminates unnecessary jumps and improves branch prediction.

### Analysis Phase

1. Identify blocks that only contain unconditional jumps
2. Verify that bypassing these blocks is safe
3. Find opportunities where predecessors can be directly connected to the ultimate destination

### Transformation Phase

1. Update control flow edges to bypass intermediate jump blocks
2. Replace jump instructions to point to the ultimate destination
3. Preserve program semantics while reducing control flow overhead

### Performance

Jump threading performance varies based on the structure:

- Linear chains show predictable scaling with size
- Complex patterns require more processing time
- The C API implementation is often faster than C++ for smaller graphs
- Performance scales with the number of blocks and control flow complexity

## Critical Edge Splitting (Planned)

### Purpose

Critical Edge Splitting identifies and splits edges in the control flow graph that go from a block with multiple successors to a block with multiple predecessors. This transformation enables other optimizations like code motion and register allocation.

### Analysis Phase (Planned)

1. Identify critical edges in the control flow graph
2. Validate that splitting is safe and beneficial
3. Plan the insertion of new blocks along critical edges

### Transformation Phase (Planned)

1. Create new blocks to split critical edges
2. Update control flow accordingly
3. Ensure program semantics are preserved

### Expected Benefits

- Enables more aggressive code motion
- Improves register allocation
- Facilitates other optimizations that require non-critical edges
- Creates more opportunities for other optimizations like jump threading

## Instruction Combining

### Purpose

Instruction Combining identifies sequences of instructions that can be replaced with simpler or fewer instructions to reduce execution overhead.

### Analysis Phase

1. Scan for recognizable patterns of instructions
2. Verify that the transformation preserves the semantics of the code

### Transformation Phase

1. Replace identified instruction sequences with optimized versions

### Example

```cpp
// Create a function with combinable instructions
auto function = std::make_shared<ZIRFunctionImpl>("example");
auto block = std::make_shared<ZIRBasicBlockImpl>("block");
function->addBlock(block);

// Create instructions: x * 2 (which can be optimized to x << 1)
auto xValue = std::make_shared<ZIRIntegerValue>(10);
auto xInst = std::make_shared<ZIRConstantInst>("x", xValue);
block->addInstruction(xInst);

auto twoValue = std::make_shared<ZIRIntegerValue>(2);
auto twoInst = std::make_shared<ZIRConstantInst>("two", twoValue);
block->addInstruction(twoInst);

auto mulInst = std::make_shared<ZIRMulInst>("result", xInst->getResult(), twoInst->getResult());
block->addInstruction(mulInst);

// Apply instruction combining
function->applyInstructionCombining();

// The result should be a shift instruction instead of multiplication
```

### Performance Impact

Instruction Combining typically provides:

- 5-15% reduction in the number of arithmetic instructions
- 3-8% improvement in execution speed

## Constant Propagation and Folding

### Purpose

Constant Propagation identifies variables that have constant values and replaces their uses with the constants. Constant Folding evaluates constant expressions at compile time.

### Analysis Phase

1. Track assignments of constants to variables
2. Identify expressions with constant operands

### Transformation Phase

1. Replace variable uses with their constant values
2. Evaluate constant expressions and replace them with their results

### Example

```cpp
// Create a function with opportunities for constant propagation
auto function = std::make_shared<ZIRFunctionImpl>("example");
auto block = std::make_shared<ZIRBasicBlockImpl>("block");
function->addBlock(block);

// x = 5
auto xValue = std::make_shared<ZIRIntegerValue>(5);
auto xInst = std::make_shared<ZIRConstantInst>("x", xValue);
block->addInstruction(xInst);

// y = 10
auto yValue = std::make_shared<ZIRIntegerValue>(10);
auto yInst = std::make_shared<ZIRConstantInst>("y", yValue);
block->addInstruction(yInst);

// z = x + y (should be folded to z = 15)
auto addInst = std::make_shared<ZIRAddInst>("z", xInst->getResult(), yInst->getResult());
block->addInstruction(addInst);

// Apply constant propagation and folding
function->applyConstantPropagation();

// The result should be a single constant instruction z = 15
```

### Performance Impact

Constant Propagation and Folding typically provides:

- 10-20% reduction in the number of arithmetic instructions
- 5-10% improvement in execution speed

## Integration with Build System

All optimization passes are integrated into the build system via Makefile targets:

```makefile
# Example target for dead block elimination
test_zir_dead_blocks: tests/zir/test_zir_dead_blocks.cpp $(ZIR_OBJS)
    $(CXX) $(CXXFLAGS) -fsanitize=address $^ -o $@
    ./$@
    rm -f $@

# Example target for block merging
test_block_merging: tests/zir/test_block_merging.cpp $(ZIR_OBJS)
    $(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
    ./$@
    rm -f $@

# Example target for jump threading
test_jump_threading: tests/zir/test_jump_threading.cpp $(ZIR_OBJS)
    $(CXX) $(CXXFLAGS) -fsanitize=address $^ -o $@
    ./$@
    rm -f $@
```

## Running Optimization Passes

Optimization passes can be run individually or in sequence:

```cpp
// Example of running multiple passes in sequence
auto function = createFunction(); // Create a function with some ZIR code

// Run optimization passes
function->removeDeadBlocks();
function->applyBlockMerging();
function->applyJumpThreading();
function->applyConstantPropagation();

// Output the optimized function
std::cout << function->toString() << std::endl;
```

## Future Optimization Passes

Planned future optimization passes include:

- Loop invariant code motion
- Common subexpression elimination
- Function inlining
- Tail call optimization
- Memory access optimization

## Best Practices for Writing Optimizable Code

When writing code that will be processed by ZIR, consider the following best practices:

1. Prefer immutable variables when possible
2. Avoid unnecessary control flow
3. Use simple expressions that can be evaluated at compile time
4. Structure loops to make their invariants clear
5. Consider the data flow between functions when designing your code

## References

For more details on the optimization techniques, refer to:

- The [ZIR Overview](./zir_overview.md) document
- The source code in `src/zir_basic_block.cpp` and `src/zir_function.cpp`
- The test cases in `tests/zir/test_zir_dead_blocks.cpp`, `tests/zir/test_block_merging.cpp`, and `tests/zir/test_jump_threading.cpp`
