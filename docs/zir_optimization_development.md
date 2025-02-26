# Developing ZIR Optimization Passes

## Overview

This guide provides developers with the information needed to create new optimization passes for the ZIR (Zacklang Intermediate Representation) system. It covers the optimization pass structure, analysis and transformation phases, testing, and integration with the existing codebase.

## Optimization Pass Structure

A well-designed ZIR optimization pass typically consists of two main phases:

1. **Analysis Phase**: Examines the ZIR code to identify optimization opportunities without modifying the code
2. **Transformation Phase**: Applies changes to the ZIR code based on the analysis results

### Best Practices for Structure

- Keep analysis and transformation clearly separated
- Make analysis methods const whenever possible
- Return detailed information from analysis to inform transformation
- Design transformations to be idempotent when possible
- Add safety checks before transformation

## Creating a New Optimization Pass

### Step 1: Define the Optimization Goal

Start by clearly defining what the optimization aims to achieve:

- What inefficiency is being targeted?
- What pattern of code will be transformed?
- What are the expected improvements?

### Step 2: Implement the Analysis Phase

Add analysis methods to the appropriate class(es), typically `ZIRBasicBlockImpl` or `ZIRFunctionImpl`:

```cpp
// Example: Analysis method in ZIRBasicBlockImpl
std::vector<Opportunity> ZIRBasicBlockImpl::findOptimizationOpportunities() const {
    std::vector<Opportunity> opportunities;

    // Analyze this block for optimization opportunities
    // ...

    return opportunities;
}

// Example: Analysis method in ZIRFunctionImpl
std::vector<std::shared_ptr<ZIRBasicBlockImpl>> ZIRFunctionImpl::findCandidateBlocks() const {
    std::vector<std::shared_ptr<ZIRBasicBlockImpl>> candidates;

    // Find blocks that are candidates for optimization
    // ...

    return candidates;
}
```

### Step 3: Implement the Transformation Phase

Add transformation methods that apply the optimization:

```cpp
// Example: Transformation method in ZIRBasicBlockImpl
bool ZIRBasicBlockImpl::applyOptimization(const Opportunity& opportunity) {
    // Apply the optimization to this block
    // ...

    return true; // Return success/failure
}

// Example: Transformation method in ZIRFunctionImpl
size_t ZIRFunctionImpl::applyOptimizationPass() {
    size_t optimizationsApplied = 0;

    // Find opportunities
    auto candidates = findCandidateBlocks();

    // Apply transformations
    for (auto& block : candidates) {
        auto opportunities = block->findOptimizationOpportunities();
        for (const auto& opportunity : opportunities) {
            if (block->applyOptimization(opportunity)) {
                optimizationsApplied++;
            }
        }
    }

    return optimizationsApplied;
}
```

### Step 4: Add C API Support

Extend the C API to make the optimization available:

```cpp
// Add to zir_c_api.h
size_t ZIRFunctionApplyOptimizationPass(ZIRFunctionRef function);

// Add to zir_c_api.cpp
size_t ZIRFunctionApplyOptimizationPass(ZIRFunctionRef function) {
    auto func = static_cast<zir::ZIRFunctionImpl*>(function);
    return func->applyOptimizationPass();
}
```

## Example: Developing a Constant Propagation Pass

Let's walk through the development of a simple constant propagation pass:

### 1. Define the Optimization Goal

**Goal**: Identify variables that are assigned constant values and replace their uses with the constants directly.

### 2. Implement Analysis

```cpp
// In ZIRBasicBlockImpl
struct ConstantAssignment {
    std::string variable;
    std::shared_ptr<ZIRValue> constantValue;
};

std::vector<ConstantAssignment> ZIRBasicBlockImpl::findConstantAssignments() const {
    std::vector<ConstantAssignment> assignments;

    for (const auto& instr : m_instructions) {
        // Look for store instructions with constant values
        if (instr->getOpcode() == ZIROpcode::Store) {
            auto storeInst = std::static_pointer_cast<ZIRStoreInst>(instr);
            auto pointer = storeInst->getPointer();
            auto value = storeInst->getValue();

            // Check if storing a constant value
            if (value->isConstant()) {
                // Find the variable name from the pointer
                std::string varName = pointer->getName();

                assignments.push_back({varName, value});
            }
        }
    }

    return assignments;
}
```

### 3. Implement Transformation

```cpp
// In ZIRBasicBlockImpl
size_t ZIRBasicBlockImpl::propagateConstants() {
    size_t replacements = 0;

    // Get constant assignments
    auto assignments = findConstantAssignments();

    // Create a map for easy lookup
    std::unordered_map<std::string, std::shared_ptr<ZIRValue>> constantMap;
    for (const auto& assignment : assignments) {
        constantMap[assignment.variable] = assignment.constantValue;
    }

    // Replace uses of constants
    for (auto& instr : m_instructions) {
        // For each operand that's a load from a variable
        for (size_t i = 0; i < instr->getOperandCount(); i++) {
            auto operand = instr->getOperand(i);

            // Check if this is a load instruction result
            if (operand->isFromInstruction()) {
                auto sourceInstr = operand->getSourceInstruction();
                if (sourceInstr && sourceInstr->getOpcode() == ZIROpcode::Load) {
                    auto loadInst = std::static_pointer_cast<ZIRLoadInst>(sourceInstr);
                    auto pointer = loadInst->getPointer();
                    std::string varName = pointer->getName();

                    // If we have a constant for this variable
                    if (constantMap.find(varName) != constantMap.end()) {
                        // Replace the operand with the constant
                        instr->replaceOperand(i, constantMap[varName]);
                        replacements++;
                    }
                }
            }
        }
    }

    return replacements;
}

// In ZIRFunctionImpl
size_t ZIRFunctionImpl::propagateConstants() {
    size_t totalReplacements = 0;

    for (auto& block : m_blocks) {
        totalReplacements += block->propagateConstants();
    }

    return totalReplacements;
}
```

### 4. Add C API Support

```cpp
// Add to zir_c_api.h
size_t ZIRFunctionPropagateConstants(ZIRFunctionRef function);

// Add to zir_c_api.cpp
size_t ZIRFunctionPropagateConstants(ZIRFunctionRef function) {
    auto func = static_cast<zir::ZIRFunctionImpl*>(function);
    return func->propagateConstants();
}
```

## Testing Optimization Passes

### Creating Test Cases

Create test files in the `tests/zir/` directory to verify your optimization pass:

```cpp
// Example: tests/zir/test_constant_propagation.cpp
#include "../../include/zir_basic_block.hpp"
#include "../../include/zir_function.hpp"
#include <cassert>
#include <iostream>
#include <memory>

using namespace zir;

int main() {
    std::cout << "Running constant propagation tests...\n";

    // Create a function with constant assignments
    auto function = std::make_shared<ZIRFunctionImpl>("test_function");
    auto block = std::make_shared<ZIRBasicBlockImpl>("entry");
    function->addBlock(block);

    // Create instructions for: x = 5; y = x + 2;
    auto allocaX = std::make_shared<ZIRAllocaInst>("x.addr", ZIRType::Kind::Integer);
    auto constVal = std::make_shared<ZIRIntegerValue>(5);
    auto storeX = std::make_shared<ZIRStoreInst>("", allocaX->getResult(), constVal);
    block->addInstruction(allocaX);
    block->addInstruction(storeX);

    auto allocaY = std::make_shared<ZIRAllocaInst>("y.addr", ZIRType::Kind::Integer);
    auto loadX = std::make_shared<ZIRLoadInst>("x.val", allocaX->getResult());
    auto const2 = std::make_shared<ZIRIntegerValue>(2);
    auto addInst = std::make_shared<ZIRAddInst>("add.result", loadX->getResult(), const2);
    auto storeY = std::make_shared<ZIRStoreInst>("", allocaY->getResult(), addInst->getResult());
    block->addInstruction(allocaY);
    block->addInstruction(loadX);
    block->addInstruction(addInst);
    block->addInstruction(storeY);

    // Before optimization, check that addInst uses loadX
    assert(addInst->getOperand(0) == loadX->getResult());

    // Apply constant propagation
    size_t replacements = function->propagateConstants();
    std::cout << "Applied " << replacements << " constant replacements\n";

    // After optimization, check that addInst uses constVal
    assert(addInst->getOperand(0) == constVal);

    std::cout << "Constant propagation tests passed!\n";
    return 0;
}
```

### Adding a Makefile Target

Add a target to the Makefile to compile and run your test:

```makefile
# Add constant propagation test target
.PHONY: test_constant_propagation
test_constant_propagation: tests/zir/test_constant_propagation.cpp $(ZIR_OBJS)
    $(CXX) $(CXXFLAGS) -fsanitize=address $^ -o $@
    ./$@
    rm -f $@
```

## Benchmarking Optimization Passes

Create benchmarks to measure the impact of your optimization:

```cpp
// Example: tests/zir/benchmarks/test_constant_propagation_bench.cpp
#include "../../../include/zir_basic_block.hpp"
#include "../../../include/zir_function.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <vector>

using namespace zir;

// Create a test function with many constant opportunities
std::shared_ptr<ZIRFunctionImpl> createTestFunction(int size) {
    auto function = std::make_shared<ZIRFunctionImpl>("benchmark_function");
    auto block = std::make_shared<ZIRBasicBlockImpl>("entry");
    function->addBlock(block);

    std::vector<std::shared_ptr<ZIRAllocaInst>> vars;
    std::vector<std::shared_ptr<ZIRLoadInst>> loads;

    // Create many variables with constant values
    for (int i = 0; i < size; i++) {
        auto alloca = std::make_shared<ZIRAllocaInst>("var" + std::to_string(i), ZIRType::Kind::Integer);
        auto constVal = std::make_shared<ZIRIntegerValue>(i);
        auto store = std::make_shared<ZIRStoreInst>("", alloca->getResult(), constVal);
        block->addInstruction(alloca);
        block->addInstruction(store);
        vars.push_back(alloca);
    }

    // Create loads from all variables
    for (auto& var : vars) {
        auto load = std::make_shared<ZIRLoadInst>(var->getResult()->getName() + ".val", var->getResult());
        block->addInstruction(load);
        loads.push_back(load);
    }

    // Create many instructions using the loads
    for (int i = 0; i < size - 1; i++) {
        auto add = std::make_shared<ZIRAddInst>("add" + std::to_string(i),
                                               loads[i]->getResult(), loads[i+1]->getResult());
        block->addInstruction(add);
    }

    return function;
}

int main() {
    // Test sizes
    std::vector<int> sizes = {100, 1000, 10000};

    for (int size : sizes) {
        std::cout << "Benchmarking constant propagation with size " << size << "...\n";

        auto function = createTestFunction(size);

        // Measure time to run optimization
        auto start = std::chrono::high_resolution_clock::now();
        size_t replacements = function->propagateConstants();
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> duration = end - start;

        std::cout << "Size: " << size << "\n";
        std::cout << "Replacements: " << replacements << "\n";
        std::cout << "Time: " << duration.count() << " ms\n\n";
    }

    return 0;
}
```

## Best Practices for Optimization Development

### Code Structure

- Keep analysis methods separate from transformation methods
- Return rich data structures from analysis methods
- Use const correctness for analysis methods
- Make transformations atomic where possible

### Performance

- Cache analysis results when appropriate
- Avoid redundant computations
- Consider memory usage for large functions
- Profile your optimization pass with large inputs

### Safety

- Verify your transformations preserve program semantics
- Add assertions to catch invalid states
- Run tests with address sanitizers
- Test edge cases thoroughly

### Maintainability

- Document the purpose of each analysis and transformation
- Add comments explaining complex algorithms
- Use meaningful names for methods and variables
- Add debug logging for tracing optimization steps

## Debugging Optimization Passes

### Useful Techniques

1. **Add logging to trace the optimization process**:

```cpp
void ZIRBasicBlockImpl::debug_printInstructions() const {
    std::cout << "Block: " << m_name << "\n";
    for (const auto& instr : m_instructions) {
        std::cout << "  " << instr->toString() << "\n";
    }
}
```

2. **Add visualization of the control flow graph**:

```cpp
void ZIRFunctionImpl::debug_printCFG() const {
    std::cout << "CFG for function: " << m_name << "\n";
    for (const auto& block : m_blocks) {
        std::cout << block->getName() << " -> ";
        for (const auto& succ : block->getSuccessors()) {
            std::cout << succ->getName() << ", ";
        }
        std::cout << "\n";
    }
}
```

3. **Use assertions to catch invalid states**:

```cpp
bool ZIRBasicBlockImpl::isValidSuccessor(std::shared_ptr<ZIRBasicBlockImpl> block) const {
    // Validate that block is a valid successor
    assert(block != nullptr && "Null successor");
    assert(block->getParent() == getParent() && "Successor from different function");
    return true;
}
```

## Integrating with Existing Passes

When developing a new optimization pass, consider how it interacts with existing passes:

### Pass Ordering

Some optimizations are more effective when run before or after others:

```cpp
// Example of running passes in an effective order
void ZIRFunctionImpl::runOptimizationPipeline() {
    // First, eliminate dead code
    removeDeadBlocks();

    // Then propagate constants
    propagateConstants();

    // Then merge blocks
    applyBlockMerging();

    // Finally, thread jumps
    applyJumpThreading();
}
```

### Optimization Pipelines

Create different optimization pipelines for different scenarios:

```cpp
// Add functions to zir_function.hpp
void ZIRFunctionImpl::applyO1Optimizations(); // Basic optimizations
void ZIRFunctionImpl::applyO2Optimizations(); // Intermediate optimizations
void ZIRFunctionImpl::applyO3Optimizations(); // Aggressive optimizations
```

## Documentation

Document your optimization pass thoroughly:

1. Add comments in the header file explaining the purpose of the optimization
2. Document the analysis phase, including what patterns are identified
3. Document the transformation phase, explaining how the code is changed
4. Add examples of before/after code
5. Document any limitations or edge cases
6. Add a section to `docs/zir_optimization_passes.md`

## Conclusion

Developing effective optimization passes for ZIR requires a combination of compiler theory knowledge, careful design, and thorough testing. By following the structure and best practices outlined in this guide, you can create optimizations that significantly improve the performance of Zacklang programs.

For more information, refer to:

- [ZIR Overview](./zir_overview.md)
- [ZIR Optimization Passes](./zir_optimization_passes.md)
- The existing optimization pass implementations in `src/zir_basic_block.cpp` and `src/zir_function.cpp`
