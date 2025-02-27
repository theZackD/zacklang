# ZIR C API Reference

## Overview

The ZIR C API provides a C-language interface to the Zacklang Intermediate Representation (ZIR). This API enables developers to interact with ZIR from languages other than C++, making it possible to integrate ZIR with various tools and frameworks.

## Core Types

The C API defines several opaque handle types:

```c
typedef void* ZIRFunctionRef;
typedef void* ZIRBasicBlockRef;
typedef void* ZIRValueRef;
typedef void* ZIRInstructionRef;
```

These handles act as references to the underlying C++ objects, allowing C code to manipulate ZIR objects without direct access to their implementation details.

## Function Management

### Creating and Manipulating Functions

```c
// Create a new function with the given name
ZIRFunctionRef ZIRFunctionCreate(const char* name);

// Get the name of a function
const char* ZIRFunctionGetName(ZIRFunctionRef function);

// Release a function and free its resources
void ZIRFunctionRelease(ZIRFunctionRef function);

// Get the entry block of a function
ZIRBasicBlockRef ZIRFunctionGetEntryBlock(ZIRFunctionRef function);

// Set the entry block of a function
void ZIRFunctionSetEntryBlock(ZIRFunctionRef function, ZIRBasicBlockRef block);

// Add a block to a function
void ZIRFunctionAddBlock(ZIRFunctionRef function, ZIRBasicBlockRef block);

// Get the number of blocks in a function
size_t ZIRFunctionGetBlockCount(ZIRFunctionRef function);
```

### Example: Creating a Function and Adding Blocks

```c
// Create a new function
ZIRFunctionRef func = ZIRFunctionCreate("example");

// Create some blocks
ZIRBasicBlockRef entry = ZIRBasicBlockCreate("entry");
ZIRBasicBlockRef body = ZIRBasicBlockCreate("body");
ZIRBasicBlockRef exit = ZIRBasicBlockCreate("exit");

// Add blocks to the function
ZIRFunctionAddBlock(func, entry);
ZIRFunctionAddBlock(func, body);
ZIRFunctionAddBlock(func, exit);

// Set the entry block
ZIRFunctionSetEntryBlock(func, entry);

// Use the function
// ...

// Clean up
ZIRFunctionRelease(func);
```

## Basic Block Management

### Creating and Manipulating Blocks

```c
// Create a new basic block with the given name
ZIRBasicBlockRef ZIRBasicBlockCreate(const char* name);

// Get the name of a basic block
const char* ZIRBasicBlockGetName(ZIRBasicBlockRef block);

// Release a basic block and free its resources
void ZIRBasicBlockRelease(ZIRBasicBlockRef block);

// Add an instruction to a block
void ZIRBasicBlockAddInstruction(ZIRBasicBlockRef block, ZIRInstructionRef instruction);

// Get the number of instructions in a block
size_t ZIRBasicBlockGetInstructionCount(ZIRBasicBlockRef block);

// Add a predecessor block
void ZIRBasicBlockAddPredecessor(ZIRBasicBlockRef block, ZIRBasicBlockRef predecessor);

// Add a successor block
void ZIRBasicBlockAddSuccessor(ZIRBasicBlockRef block, ZIRBasicBlockRef successor);

// Get the number of predecessors
size_t ZIRBasicBlockGetPredecessorCount(ZIRBasicBlockRef block);

// Get the number of successors
size_t ZIRBasicBlockGetSuccessorCount(ZIRBasicBlockRef block);
```

### Example: Creating a Control Flow Graph

```c
// Create blocks
ZIRBasicBlockRef entry = ZIRBasicBlockCreate("entry");
ZIRBasicBlockRef then_block = ZIRBasicBlockCreate("then");
ZIRBasicBlockRef else_block = ZIRBasicBlockCreate("else");
ZIRBasicBlockRef exit = ZIRBasicBlockCreate("exit");

// Link blocks
ZIRBasicBlockAddSuccessor(entry, then_block);
ZIRBasicBlockAddSuccessor(entry, else_block);
ZIRBasicBlockAddPredecessor(then_block, entry);
ZIRBasicBlockAddPredecessor(else_block, entry);

ZIRBasicBlockAddSuccessor(then_block, exit);
ZIRBasicBlockAddSuccessor(else_block, exit);
ZIRBasicBlockAddPredecessor(exit, then_block);
ZIRBasicBlockAddPredecessor(exit, else_block);
```

## Value Management

### Creating and Manipulating Values

```c
// Create an integer constant value
ZIRValueRef ZIRIntegerValueCreate(int64_t value);

// Create a float constant value
ZIRValueRef ZIRFloatValueCreate(double value);

// Create a boolean constant value
ZIRValueRef ZIRBooleanValueCreate(bool value);

// Create a string constant value
ZIRValueRef ZIRStringValueCreate(const char* value);

// Release a value and free its resources
void ZIRValueRelease(ZIRValueRef value);

// Get the type of a value
int ZIRValueGetType(ZIRValueRef value);
```

### Example: Creating and Using Values

```c
// Create some constant values
ZIRValueRef int_val = ZIRIntegerValueCreate(42);
ZIRValueRef float_val = ZIRFloatValueCreate(3.14);
ZIRValueRef bool_val = ZIRBooleanValueCreate(true);
ZIRValueRef str_val = ZIRStringValueCreate("Hello, ZIR!");

// Use the values with instructions
// ...

// Clean up
ZIRValueRelease(int_val);
ZIRValueRelease(float_val);
ZIRValueRelease(bool_val);
ZIRValueRelease(str_val);
```

## Instruction Management

### Creating and Manipulating Instructions

```c
// Create an alloca instruction
ZIRInstructionRef ZIRAllocaInstructionCreate(const char* result, int type);

// Create a load instruction
ZIRInstructionRef ZIRLoadInstructionCreate(const char* result, ZIRValueRef pointer);

// Create a store instruction
ZIRInstructionRef ZIRStoreInstructionCreate(ZIRValueRef pointer, ZIRValueRef value);

// Create a binary operation instruction (add, sub, mul, div)
ZIRInstructionRef ZIRBinaryInstructionCreate(int opcode, const char* result,
                                            ZIRValueRef lhs, ZIRValueRef rhs);

// Create a comparison instruction
ZIRInstructionRef ZIRComparisonInstructionCreate(int opcode, const char* result,
                                                ZIRValueRef lhs, ZIRValueRef rhs);

// Create a jump instruction
ZIRInstructionRef ZIRJumpInstructionCreate(ZIRBasicBlockRef target);

// Create a conditional branch instruction
ZIRInstructionRef ZIRCondBranchInstructionCreate(ZIRValueRef condition,
                                                ZIRBasicBlockRef true_target,
                                                ZIRBasicBlockRef false_target);

// Create a return instruction
ZIRInstructionRef ZIRReturnInstructionCreate(ZIRValueRef value);

// Release an instruction and free its resources
void ZIRInstructionRelease(ZIRInstructionRef instruction);
```

### Example: Creating Instructions for a Simple Function

```c
// Create a function and blocks
ZIRFunctionRef func = ZIRFunctionCreate("add");
ZIRBasicBlockRef entry = ZIRBasicBlockCreate("entry");
ZIRFunctionAddBlock(func, entry);

// Create parameter values
ZIRValueRef param_a = ZIRParameterValueCreate(0, "a");
ZIRValueRef param_b = ZIRParameterValueCreate(1, "b");

// Create an add instruction
ZIRInstructionRef add_inst = ZIRBinaryInstructionCreate(ZIR_OPCODE_ADD, "result", param_a, param_b);
ZIRBasicBlockAddInstruction(entry, add_inst);

// Create a return instruction
ZIRInstructionRef ret_inst = ZIRReturnInstructionCreate(ZIRInstructionGetResult(add_inst));
ZIRBasicBlockAddInstruction(entry, ret_inst);
```

## Optimization Passes

The C API provides access to ZIR's optimization passes:

```c
// Find and remove dead blocks in a function
size_t ZIRFunctionRemoveDeadBlocks(ZIRFunctionRef function);

// Merge blocks where possible
size_t ZIRFunctionMergeBlocks(ZIRFunctionRef function);

// Apply jump threading optimizations
size_t ZIRFunctionApplyJumpThreading(ZIRFunctionRef function);

// Value Numbering Functions

// Perform local value numbering on a block
ZIRValueMap* ZIRBasicBlock_performLocalValueNumbering(ZIRBasicBlock* block);

// Check if a block has redundant computations
int ZIRBasicBlock_hasRedundantComputations(ZIRBasicBlock* block);

// Find redundant computations in a block
ZIRRedundantPairs* ZIRBasicBlock_findRedundantComputations(ZIRBasicBlock* block);

// Perform global value numbering on a function
ZIRValueMap* ZIRFunction_performGlobalValueNumbering(ZIRFunction* function);

// Check if a function has global redundant computations
int ZIRFunction_hasGlobalRedundantComputations(ZIRFunction* function);

// Find global redundant computations in a function
ZIRRedundantPairs* ZIRFunction_findGlobalRedundantComputations(ZIRFunction* function);

// Helper functions for working with value numbering results
void ZIRValueMap_destroy(ZIRValueMap* value_map);
void ZIRRedundantPairs_destroy(ZIRRedundantPairs* redundant_pairs);
```

### Example: Using Value Numbering

```c
// Create a function with blocks
ZIRFunction* function = zir_create_function("example");
ZIRBasicBlock* block1 = zir_create_basic_block("block1");
zir_function_add_block(function, block1);

// Add instructions with redundant computations
ZIRValue* int5 = zir_create_int32_value(5);
ZIRValue* int10 = zir_create_int32_value(10);
ZIRInstruction* add1 = zir_create_add_instruction(int5, int10);
ZIRInstruction* add2 = zir_create_add_instruction(int5, int10); // Redundant with add1
zir_block_add_instruction(block1, add1);
zir_block_add_instruction(block1, add2);

// Perform local value numbering
ZIRValueMap* value_map = ZIRBasicBlock_performLocalValueNumbering(block1);

// Check for redundant computations
int has_redundant = ZIRBasicBlock_hasRedundantComputations(block1);
if (has_redundant) {
    // Find the redundant pairs
    ZIRRedundantPairs* pairs = ZIRBasicBlock_findRedundantComputations(block1);

    // Process the pairs
    // ...

    // Clean up
    ZIRRedundantPairs_destroy(pairs);
}

// Clean up
ZIRValueMap_destroy(value_map);
```

## Serialization and Debugging

```c
// Get a string representation of a function
const char* ZIRFunctionToString(ZIRFunctionRef function);

// Get a string representation of a basic block
const char* ZIRBasicBlockToString(ZIRBasicBlockRef block);

// Get a string representation of an instruction
const char* ZIRInstructionToString(ZIRInstructionRef instruction);

// Get a string representation of a value
const char* ZIRValueToString(ZIRValueRef value);
```

### Example: Debugging ZIR Structures

```c
// Create a function and some blocks
ZIRFunctionRef func = createFunction();

// Print the function structure
const char* func_str = ZIRFunctionToString(func);
printf("Function structure:\n%s\n", func_str);

// Get the entry block and print it
ZIRBasicBlockRef entry = ZIRFunctionGetEntryBlock(func);
const char* block_str = ZIRBasicBlockToString(entry);
printf("Entry block:\n%s\n", block_str);
```

## Memory Management

The C API follows a manual memory management model, where resources must be explicitly released when no longer needed:

```c
// Release a function
ZIRFunctionRelease(function);

// Release a block
ZIRBasicBlockRelease(block);

// Release an instruction
ZIRInstructionRelease(instruction);

// Release a value
ZIRValueRelease(value);
```

It's important to release resources in the reverse order of their creation to avoid dangling references.

## Error Handling

The C API uses return values to indicate success or failure:

```c
// Most functions return NULL on failure
ZIRFunctionRef func = ZIRFunctionCreate("example");
if (func == NULL) {
    // Handle error
}

// Functions that modify state return bool to indicate success
bool success = ZIRFunctionAddBlock(func, block);
if (!success) {
    // Handle error
}
```

## Block Merging Example

```c
// Create a function with blocks to merge
ZIRFunctionRef func = ZIRFunctionCreate("example");
ZIRBasicBlockRef block1 = ZIRBasicBlockCreate("block1");
ZIRBasicBlockRef block2 = ZIRBasicBlockCreate("block2");

// Add blocks to function
ZIRFunctionAddBlock(func, block1);
ZIRFunctionAddBlock(func, block2);

// Link blocks
ZIRBasicBlockAddSuccessor(block1, block2);
ZIRBasicBlockAddPredecessor(block2, block1);

// Add a jump from block1 to block2
ZIRInstructionRef jump = ZIRJumpInstructionCreate(block2);
ZIRBasicBlockAddInstruction(block1, jump);

// Add a simple instruction to block2
ZIRValueRef const_val = ZIRIntegerValueCreate(42);
ZIRInstructionRef const_inst = ZIRConstantInstructionCreate("result", const_val);
ZIRBasicBlockAddInstruction(block2, const_inst);

// Check if blocks can be merged
bool can_merge = ZIRBlockCanMergeWith(block1, block2);
printf("Can merge blocks: %s\n", can_merge ? "yes" : "no");

// If mergeable, perform the merge
if (can_merge) {
    bool merged = ZIRBlockMergeWith(block1, block2);
    printf("Blocks merged: %s\n", merged ? "yes" : "no");
}
```

## Jump Threading Example

```c
// Create a function with blocks for jump threading
ZIRFunctionRef func = ZIRFunctionCreate("example");
ZIRBasicBlockRef blockA = ZIRBasicBlockCreate("A");
ZIRBasicBlockRef blockB = ZIRBasicBlockCreate("B");
ZIRBasicBlockRef blockC = ZIRBasicBlockCreate("C");

// Add blocks to function
ZIRFunctionAddBlock(func, blockA);
ZIRFunctionAddBlock(func, blockB);
ZIRFunctionAddBlock(func, blockC);

// Link blocks
ZIRBasicBlockAddSuccessor(blockA, blockB);
ZIRBasicBlockAddPredecessor(blockB, blockA);
ZIRBasicBlockAddSuccessor(blockB, blockC);
ZIRBasicBlockAddPredecessor(blockC, blockB);

// Add a jump from blockA to blockB
ZIRInstructionRef jumpAB = ZIRJumpInstructionCreate(blockB);
ZIRBasicBlockAddInstruction(blockA, jumpAB);

// Add a jump from blockB to blockC
ZIRInstructionRef jumpBC = ZIRJumpInstructionCreate(blockC);
ZIRBasicBlockAddInstruction(blockB, jumpBC);

// Find and apply jump threading opportunities
ZIRJumpThreadingOpportunityRef* opportunities = ZIRFindJumpThreadingOpportunities(func);
int count = 0;
while (opportunities[count] != NULL) {
    ZIRApplyJumpThreading(opportunities[count]);
    count++;
}
printf("Applied %d jump threading optimizations\n", count);

// Free the opportunities array
ZIRReleaseJumpThreadingOpportunities(opportunities);
```

## Conclusion

The ZIR C API provides a comprehensive interface for working with the Zacklang Intermediate Representation from C code. It enables developers to create, manipulate, and optimize ZIR code without direct access to the C++ implementation.

For more information, refer to:

- The `include/zir_c_api.h` header file
- The `src/zir_c_api.cpp` source file
- The C API tests in `tests/zir/test_zir_c_api.cpp`
