# ZIR: Zacklang Intermediate Representation

## Overview

ZIR (Zacklang Intermediate Representation) is the core intermediate representation used in the Zacklang compiler infrastructure. It serves as a bridge between the Zacklang source code and target machine code or LLVM IR. ZIR provides a robust foundation for implementing various compiler optimizations and analysis passes.

## Key Components

### Basic Structure

ZIR is organized as a Control Flow Graph (CFG) with the following key components:

- **ZIRFunction**: Represents a function in the program
- **ZIRBasicBlock**: Represents a basic block, a straight-line code sequence with no branches
- **ZIRInstruction**: Represents an instruction within a basic block
- **ZIRValue**: Represents a value manipulated by instructions

### Type System

ZIR has a simple but effective type system that includes:

- Primitives (integer, float, boolean)
- String type
- Void type
- Custom composite types

## Core Classes

### ZIRBasicBlockImpl

The basic block implementation contains:

- A unique identifier
- A list of instructions
- References to predecessor and successor blocks
- A parent function reference

```cpp
// Example of a ZIRBasicBlockImpl
auto block = std::make_shared<ZIRBasicBlockImpl>("entry");
block->addInstruction(std::make_shared<SomeInstruction>());
```

### ZIRFunctionImpl

The function implementation contains:

- A unique name
- A list of basic blocks
- Entry point reference
- Return type information

```cpp
// Example of a ZIRFunctionImpl
auto function = std::make_shared<ZIRFunctionImpl>("main");
function->addBlock(entryBlock);
function->setEntryPoint(entryBlock);
```

### ZIRInstructionImpl

The base class for all instructions in ZIR:

- Has an opcode
- May produce a result
- May use operands

```cpp
// Example of a simple instruction implementation
class AddInstruction : public ZIRInstructionImpl {
public:
    AddInstruction(const std::string& result,
                  std::shared_ptr<ZIRValue> lhs,
                  std::shared_ptr<ZIRValue> rhs)
        : ZIRInstructionImpl(ZIROpcodes::Add, result) {
        addOperand(lhs);
        addOperand(rhs);
    }

    // Implementation details...
};
```

## CFG Structure

The Control Flow Graph (CFG) in ZIR connects basic blocks through explicit successor and predecessor relationships:

```
Function
 |
 ├── Entry Block
 |    └── Successor: Block 1, Block 2
 |
 ├── Block 1
 |    ├── Predecessor: Entry Block
 |    └── Successor: Exit Block
 |
 ├── Block 2
 |    ├── Predecessor: Entry Block
 |    └── Successor: Exit Block
 |
 └── Exit Block
      └── Predecessor: Block 1, Block 2
```

## C API

ZIR provides a C API for interoperability with external tools and languages:

```c
// Example of using the ZIR C API
ZIRFunctionRef function = ZIRFunctionCreate("example");
ZIRBasicBlockRef block = ZIRBasicBlockCreate("entry");
ZIRFunctionAddBlock(function, block);
```

## Memory Management

ZIR uses smart pointers for memory management, primarily `std::shared_ptr` for objects that may be referenced from multiple locations.

```cpp
// Example of ZIR memory management
auto function = std::make_shared<ZIRFunctionImpl>("function");
auto block = std::make_shared<ZIRBasicBlockImpl>("block");
function->addBlock(block);
```

## Serialization

ZIR objects can be serialized to string representations for debugging or visualization purposes:

```cpp
// Example of serialization
std::string functionStr = function->toString();
std::cout << functionStr << std::endl;
```

## Next Steps

- See [Zacklang to ZIR Translation](./zacklang_to_zir.md) for details on how Zacklang code is translated to ZIR
- See [ZIR Optimization Passes](./zir_optimization_passes.md) for information about the available optimization techniques
