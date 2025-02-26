# Zacklang to ZIR Translation

## Overview

This document explains the process of translating Zacklang source code to ZIR (Zacklang Intermediate Representation). The translation is a multi-step process that involves lexical analysis, parsing, semantic analysis, and IR generation.

## Translation Pipeline

1. **Lexical Analysis**: Converting source code into tokens
2. **Parsing**: Building an Abstract Syntax Tree (AST) from tokens
3. **Semantic Analysis**: Checking types and resolving identifiers
4. **IR Generation**: Translating the AST to ZIR

## Examples

### Simple Variable Declaration

**Zacklang Code:**

```zacklang
let x: int = 42;
```

**AST Representation:**

```
VarDecl
 ├── Name: x
 ├── Type: int
 └── Value: IntLiteral(42)
```

**ZIR Code:**

```cpp
// Creating a function
auto function = std::make_shared<ZIRFunctionImpl>("main");
auto entryBlock = std::make_shared<ZIRBasicBlockImpl>("entry");
function->addBlock(entryBlock);

// Creating a constant value
auto constVal = std::make_shared<ZIRIntegerValue>(42);

// Creating an alloca instruction for variable x
auto allocaInst = std::make_shared<ZIRAllocaInst>("x.addr", ZIRType::Kind::Integer);
entryBlock->addInstruction(allocaInst);

// Creating a store instruction
auto storeInst = std::make_shared<ZIRStoreInst>("", allocaInst->getResult(), constVal);
entryBlock->addInstruction(storeInst);
```

### Function Declaration

**Zacklang Code:**

```zacklang
fn add(a: int, b: int) -> int {
    return a + b;
}
```

**AST Representation:**

```
FunctionDecl
 ├── Name: add
 ├── Parameters
 │    ├── Parameter(a, int)
 │    └── Parameter(b, int)
 ├── ReturnType: int
 └── Body
      └── ReturnStmt
           └── BinaryExpr(+)
                ├── VarRef(a)
                └── VarRef(b)
```

**ZIR Code:**

```cpp
// Creating a function
auto function = std::make_shared<ZIRFunctionImpl>("add");
auto entryBlock = std::make_shared<ZIRBasicBlockImpl>("entry");
function->addBlock(entryBlock);

// Creating parameter allocations
auto allocaA = std::make_shared<ZIRAllocaInst>("a.addr", ZIRType::Kind::Integer);
auto allocaB = std::make_shared<ZIRAllocaInst>("b.addr", ZIRType::Kind::Integer);
entryBlock->addInstruction(allocaA);
entryBlock->addInstruction(allocaB);

// Store parameters
auto storeA = std::make_shared<ZIRStoreInst>("", allocaA->getResult(),
                                            std::make_shared<ZIRParameterValue>(0, "a"));
auto storeB = std::make_shared<ZIRStoreInst>("", allocaB->getResult(),
                                            std::make_shared<ZIRParameterValue>(1, "b"));
entryBlock->addInstruction(storeA);
entryBlock->addInstruction(storeB);

// Load values
auto loadA = std::make_shared<ZIRLoadInst>("a.val", allocaA->getResult());
auto loadB = std::make_shared<ZIRLoadInst>("b.val", allocaB->getResult());
entryBlock->addInstruction(loadA);
entryBlock->addInstruction(loadB);

// Add values
auto addInst = std::make_shared<ZIRAddInst>("add.result", loadA->getResult(), loadB->getResult());
entryBlock->addInstruction(addInst);

// Return result
auto returnInst = std::make_shared<ZIRReturnInst>(addInst->getResult());
entryBlock->addInstruction(returnInst);
```

### Conditional Statement

**Zacklang Code:**

```zacklang
fn abs(x: int) -> int {
    if x < 0 {
        return -x;
    } else {
        return x;
    }
}
```

**ZIR Code:**

```cpp
// Function and entry block
auto function = std::make_shared<ZIRFunctionImpl>("abs");
auto entryBlock = std::make_shared<ZIRBasicBlockImpl>("entry");
auto thenBlock = std::make_shared<ZIRBasicBlockImpl>("then");
auto elseBlock = std::make_shared<ZIRBasicBlockImpl>("else");
auto exitBlock = std::make_shared<ZIRBasicBlockImpl>("exit");

function->addBlock(entryBlock);
function->addBlock(thenBlock);
function->addBlock(elseBlock);
function->addBlock(exitBlock);

// Parameter allocation
auto allocaX = std::make_shared<ZIRAllocaInst>("x.addr", ZIRType::Kind::Integer);
entryBlock->addInstruction(allocaX);

// Store parameter
auto storeX = std::make_shared<ZIRStoreInst>("", allocaX->getResult(),
                                           std::make_shared<ZIRParameterValue>(0, "x"));
entryBlock->addInstruction(storeX);

// Load value
auto loadX = std::make_shared<ZIRLoadInst>("x.val", allocaX->getResult());
entryBlock->addInstruction(loadX);

// Comparison
auto zeroVal = std::make_shared<ZIRIntegerValue>(0);
auto cmpInst = std::make_shared<ZIRLtInst>("cmp.result", loadX->getResult(), zeroVal);
entryBlock->addInstruction(cmpInst);

// Branch
auto branchInst = std::make_shared<ZIRCondBranchInst>(cmpInst->getResult(), thenBlock, elseBlock);
entryBlock->addInstruction(branchInst);

// Link blocks
entryBlock->addSuccessor(thenBlock);
entryBlock->addSuccessor(elseBlock);
thenBlock->addPredecessor(entryBlock);
elseBlock->addPredecessor(entryBlock);

// Then block - negate x
auto loadXThen = std::make_shared<ZIRLoadInst>("x.then", allocaX->getResult());
thenBlock->addInstruction(loadXThen);

auto negInst = std::make_shared<ZIRNegInst>("neg.result", loadXThen->getResult());
thenBlock->addInstruction(negInst);

auto returnThen = std::make_shared<ZIRReturnInst>(negInst->getResult());
thenBlock->addInstruction(returnThen);

// Else block - return x directly
auto loadXElse = std::make_shared<ZIRLoadInst>("x.else", allocaX->getResult());
elseBlock->addInstruction(loadXElse);

auto returnElse = std::make_shared<ZIRReturnInst>(loadXElse->getResult());
elseBlock->addInstruction(returnElse);
```

### Loop Statement

**Zacklang Code:**

```zacklang
fn sum(n: int) -> int {
    let result: int = 0;
    let i: int = 1;
    while i <= n {
        result = result + i;
        i = i + 1;
    }
    return result;
}
```

**ZIR Code:**

```cpp
// Function and blocks
auto function = std::make_shared<ZIRFunctionImpl>("sum");
auto entryBlock = std::make_shared<ZIRBasicBlockImpl>("entry");
auto condBlock = std::make_shared<ZIRBasicBlockImpl>("cond");
auto bodyBlock = std::make_shared<ZIRBasicBlockImpl>("body");
auto exitBlock = std::make_shared<ZIRBasicBlockImpl>("exit");

function->addBlock(entryBlock);
function->addBlock(condBlock);
function->addBlock(bodyBlock);
function->addBlock(exitBlock);

// Parameter and variable allocations
auto allocaN = std::make_shared<ZIRAllocaInst>("n.addr", ZIRType::Kind::Integer);
auto allocaResult = std::make_shared<ZIRAllocaInst>("result.addr", ZIRType::Kind::Integer);
auto allocaI = std::make_shared<ZIRAllocaInst>("i.addr", ZIRType::Kind::Integer);
entryBlock->addInstruction(allocaN);
entryBlock->addInstruction(allocaResult);
entryBlock->addInstruction(allocaI);

// Store parameter
auto storeN = std::make_shared<ZIRStoreInst>("", allocaN->getResult(),
                                           std::make_shared<ZIRParameterValue>(0, "n"));
entryBlock->addInstruction(storeN);

// Initialize result = 0
auto zeroVal = std::make_shared<ZIRIntegerValue>(0);
auto storeResult = std::make_shared<ZIRStoreInst>("", allocaResult->getResult(), zeroVal);
entryBlock->addInstruction(storeResult);

// Initialize i = 1
auto oneVal = std::make_shared<ZIRIntegerValue>(1);
auto storeI = std::make_shared<ZIRStoreInst>("", allocaI->getResult(), oneVal);
entryBlock->addInstruction(storeI);

// Jump to condition
auto jumpToCond = std::make_shared<ZIRJumpInst>(condBlock);
entryBlock->addInstruction(jumpToCond);
entryBlock->addSuccessor(condBlock);
condBlock->addPredecessor(entryBlock);

// Condition block - check i <= n
auto loadI = std::make_shared<ZIRLoadInst>("i.val", allocaI->getResult());
auto loadN = std::make_shared<ZIRLoadInst>("n.val", allocaN->getResult());
condBlock->addInstruction(loadI);
condBlock->addInstruction(loadN);

auto cmpInst = std::make_shared<ZIRLeInst>("cmp.result", loadI->getResult(), loadN->getResult());
condBlock->addInstruction(cmpInst);

auto branchInst = std::make_shared<ZIRCondBranchInst>(cmpInst->getResult(), bodyBlock, exitBlock);
condBlock->addInstruction(branchInst);
condBlock->addSuccessor(bodyBlock);
condBlock->addSuccessor(exitBlock);
bodyBlock->addPredecessor(condBlock);
exitBlock->addPredecessor(condBlock);

// Body block - update result and i
auto loadIBody = std::make_shared<ZIRLoadInst>("i.body", allocaI->getResult());
auto loadResult = std::make_shared<ZIRLoadInst>("result.val", allocaResult->getResult());
bodyBlock->addInstruction(loadIBody);
bodyBlock->addInstruction(loadResult);

// result = result + i
auto addResult = std::make_shared<ZIRAddInst>("add.result", loadResult->getResult(),
                                             loadIBody->getResult());
bodyBlock->addInstruction(addResult);
auto storeNewResult = std::make_shared<ZIRStoreInst>("", allocaResult->getResult(),
                                                    addResult->getResult());
bodyBlock->addInstruction(storeNewResult);

// i = i + 1
auto addI = std::make_shared<ZIRAddInst>("add.i", loadIBody->getResult(), oneVal);
bodyBlock->addInstruction(addI);
auto storeNewI = std::make_shared<ZIRStoreInst>("", allocaI->getResult(), addI->getResult());
bodyBlock->addInstruction(storeNewI);

// Jump back to condition
auto jumpBackToCond = std::make_shared<ZIRJumpInst>(condBlock);
bodyBlock->addInstruction(jumpBackToCond);
bodyBlock->addSuccessor(condBlock);
condBlock->addPredecessor(bodyBlock);

// Exit block - return result
auto loadFinalResult = std::make_shared<ZIRLoadInst>("final.result", allocaResult->getResult());
exitBlock->addInstruction(loadFinalResult);
auto returnInst = std::make_shared<ZIRReturnInst>(loadFinalResult->getResult());
exitBlock->addInstruction(returnInst);
```

## Key Translation Patterns

### Variables

- Variables are translated to alloca instructions followed by store instructions
- Variable references are translated to load instructions

### Expressions

- Arithmetic expressions are translated to corresponding arithmetic instructions
- Comparison expressions are translated to comparison instructions
- Function calls are translated to call instructions

### Control Flow

- If statements are translated to conditional branches with then/else blocks
- While loops are translated to condition blocks and body blocks with backedges
- For loops are decomposed into their while loop equivalents

## Memory Management in Translation

During translation, ZIR objects are created using shared pointers:

```cpp
auto function = std::make_shared<ZIRFunctionImpl>("function_name");
```

The ownership of these objects is managed through parent-child relationships:

- Functions own blocks
- Blocks own instructions
- Instructions own values

## Optimizations During Translation

Some simple optimizations can be applied during the translation phase:

- Constant folding for compile-time evaluable expressions
- Dead code elimination for unreachable code
- Simple strength reduction

More complex optimizations are performed on the ZIR after the translation is complete. See [ZIR Optimization Passes](./zir_optimization_passes.md) for details.
