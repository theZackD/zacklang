#include "../include/zir_function.hpp"
#include "../include/zir_c_api.h"
#include "../include/zir_arithmetic.hpp"
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <queue>

namespace zir
{
    std::atomic<uint64_t> ZIRFunctionImpl::next_id{0};

    void ZIRFunctionImpl::addBlock(std::shared_ptr<ZIRBasicBlockImpl> block)
    {
        if (!block)
        {
            throw std::invalid_argument("Cannot add null block to function");
        }

        std::cout << "Debug: Adding block to function " << name << std::endl;
        std::cout << "Debug: Block parent before: " << block->getParentFunction() << std::endl;

        // Step 1: Clear previous parent if any
        if (auto parent = block->getParentFunction())
        {
            std::cout << "Debug: Clearing previous parent" << std::endl;
            block->setParentFunction(nullptr);
        }

        // Step 2: Add block to our list
        std::cout << "Debug: Adding block to list" << std::endl;
        blocks.push_back(block);

        // Step 3: Set ourselves as the parent
        block->setParentFunction(this);

        std::cout << "Debug: Block parent after: " << block->getParentFunction() << std::endl;
    }

    void ZIRFunctionImpl::removeBlock(std::shared_ptr<ZIRBasicBlockImpl> block)
    {
        if (!block)
        {
            return;
        }

        std::cout << "Debug: Removing block from function " << name << std::endl;
        std::cout << "Debug: Block parent before: " << block->getParentFunction() << std::endl;

        auto it = std::find(blocks.begin(), blocks.end(), block);
        if (it != blocks.end())
        {
            // Only clear parent if we are the parent
            if (block->getParentFunction() == this)
            {
                std::cout << "Debug: Clearing parent pointer" << std::endl;
                block->setParentFunction(nullptr);
            }
            std::cout << "Debug: Removing block from list" << std::endl;
            blocks.erase(it);
        }

        std::cout << "Debug: Block parent after: " << block->getParentFunction() << std::endl;
    }

    std::shared_ptr<ZIRBasicBlockImpl> ZIRFunctionImpl::getBlock(size_t index) const
    {
        if (index >= blocks.size())
        {
            return nullptr;
        }
        return blocks[index];
    }

    std::vector<std::shared_ptr<ZIRBasicBlockImpl>> ZIRFunctionImpl::findDeadBlocks() const
    {
        std::vector<std::shared_ptr<ZIRBasicBlockImpl>> deadBlocks;

        // Empty function has no dead blocks
        if (blocks.empty())
        {
            return deadBlocks;
        }

        // Consider the first block as the entry block
        auto entryBlock = blocks[0];

        // Check each block for reachability from the entry block
        for (const auto &block : blocks)
        {
            // Skip the entry block itself
            if (block == entryBlock)
            {
                continue;
            }

            // If a block is not reachable from the entry block, it's dead
            if (!block->isReachableFrom(entryBlock))
            {
                deadBlocks.push_back(block);
            }
        }

        return deadBlocks;
    }

    size_t ZIRFunctionImpl::removeDeadBlocks()
    {
        // Find all dead blocks
        auto deadBlocks = findDeadBlocks();
        size_t removedCount = 0;

        // Remove each dead block
        for (const auto &block : deadBlocks)
        {
            std::cout << "Debug: Removing dead block: " << block->getName() << std::endl;

            // Remove all references to this block from other blocks' successor/predecessor lists
            for (const auto &otherBlock : blocks)
            {
                if (otherBlock != block)
                {
                    otherBlock->removePredecessor(block);
                    otherBlock->removeSuccessor(block);
                }
            }

            // Remove the block from the function
            removeBlock(block);
            removedCount++;
        }

        return removedCount;
    }

    // Global value numbering implementation
    std::unordered_map<std::string, size_t> ZIRFunctionImpl::performGlobalValueNumbering()
    {
        // Maps variables to their value numbers across the entire function
        std::unordered_map<std::string, size_t> globalValueMap;

        // Map to track arithmetic expressions and their value numbers
        std::unordered_map<std::string, size_t> expressionToValueNumber;

        // Track the next available value number
        size_t nextValueNumber = 0;

        // Create a map of all instructions by their result for quick lookup
        std::unordered_map<std::string, std::shared_ptr<ZIRInstructionImpl>> resultToInstruction;

        // First pass: collect all instructions
        for (const auto &block : blocks)
        {
            for (size_t i = 0; i < block->getInstructionCount(); i++)
            {
                auto instr = block->getInstruction(i);
                if (!instr || instr->getResult().empty())
                    continue;

                resultToInstruction[instr->getResult()] = instr;
            }
        }

        std::cout << "DEBUG: Global Value Numbering - Found " << resultToInstruction.size() << " instructions with results" << std::endl;

        // Second pass: perform global value numbering
        for (const auto &[result, instr] : resultToInstruction)
        {
            // Skip if we've already assigned a value number
            if (globalValueMap.find(result) != globalValueMap.end())
            {
                continue;
            }

            ZIROpcode opcode = instr->getOpcode();

            // Handle NOP instructions
            if (opcode == ZIROpcode::NOP)
            {
                globalValueMap[result] = nextValueNumber++;
                std::cout << "DEBUG: NOP instruction " << result << " assigned value number " << globalValueMap[result] << std::endl;
            }
            // Handle arithmetic operations
            else if (opcode == ZIROpcode::ADD ||
                     opcode == ZIROpcode::SUB ||
                     opcode == ZIROpcode::MUL ||
                     opcode == ZIROpcode::DIV)
            {

                auto binOp = std::dynamic_pointer_cast<BinaryArithmeticInst>(instr);
                if (!binOp)
                {
                    globalValueMap[result] = nextValueNumber++;
                    std::cout << "DEBUG: Failed to cast to BinaryArithmeticInst: " << result << std::endl;
                    continue;
                }

                auto left = binOp->getLeft();
                auto right = binOp->getRight();

                if (!left || !right)
                {
                    globalValueMap[result] = nextValueNumber++;
                    std::cout << "DEBUG: Missing operands for " << result << std::endl;
                    continue;
                }

                // Create a canonical expression string
                std::string leftStr = left->toString();
                std::string rightStr = right->toString();
                std::string exprStr;

                // For commutative operations, order operands consistently
                if (opcode == ZIROpcode::ADD || opcode == ZIROpcode::MUL)
                {
                    if (leftStr > rightStr)
                    {
                        exprStr = std::to_string(static_cast<int>(opcode)) + ":" + rightStr + ":" + leftStr;
                    }
                    else
                    {
                        exprStr = std::to_string(static_cast<int>(opcode)) + ":" + leftStr + ":" + rightStr;
                    }
                }
                else
                {
                    // Non-commutative operations
                    exprStr = std::to_string(static_cast<int>(opcode)) + ":" + leftStr + ":" + rightStr;
                }

                std::cout << "DEBUG: " << result << " has expression " << exprStr << std::endl;

                // Check if we've seen this expression before
                if (expressionToValueNumber.find(exprStr) != expressionToValueNumber.end())
                {
                    // Reuse the same value number
                    globalValueMap[result] = expressionToValueNumber[exprStr];
                    std::cout << "DEBUG: Reusing value number " << globalValueMap[result] << " for " << result << std::endl;
                }
                else
                {
                    // Create a new value number
                    globalValueMap[result] = nextValueNumber;
                    expressionToValueNumber[exprStr] = nextValueNumber;
                    std::cout << "DEBUG: New value number " << nextValueNumber << " for " << result << std::endl;
                    nextValueNumber++;
                }
            }
            // Other instruction types get unique value numbers
            else
            {
                globalValueMap[result] = nextValueNumber++;
                std::cout << "DEBUG: Other instruction " << result << " assigned value number " << globalValueMap[result] << std::endl;
            }
        }

        std::cout << "DEBUG: Final global value map:" << std::endl;
        for (const auto &[result, vn] : globalValueMap)
        {
            std::cout << "  " << result << " -> " << vn << std::endl;
        }

        return globalValueMap;
    }

    // Check if the function has any global redundant computations
    bool ZIRFunctionImpl::hasGlobalRedundantComputations() const
    {
        // Use the findGlobalRedundantComputations method to check if there are any redundant pairs
        auto pairs = findGlobalRedundantComputations();
        return !pairs.empty();
    }

    // Find globally redundant computations across all blocks
    std::vector<std::pair<std::shared_ptr<ZIRInstructionImpl>, std::shared_ptr<ZIRInstructionImpl>>>
    ZIRFunctionImpl::findGlobalRedundantComputations() const
    {
        std::vector<std::pair<std::shared_ptr<ZIRInstructionImpl>, std::shared_ptr<ZIRInstructionImpl>>> redundantPairs;

        // Maps to track expressions across all blocks
        std::unordered_map<std::string, std::shared_ptr<ZIRInstructionImpl>> expressionToInstruction;
        std::unordered_map<std::string, std::shared_ptr<ZIRInstructionImpl>> nopResultToInstruction;

        // Process each block
        for (const auto &block : blocks)
        {
            // Process each instruction in the block
            for (size_t i = 0; i < block->getInstructionCount(); i++)
            {
                auto instr = block->getInstruction(i);
                if (!instr)
                    continue;

                // Get instruction data
                std::string result = instr->getResult();
                if (result.empty())
                    continue;

                ZIROpcode opcode = instr->getOpcode();

                // Handle NOP instructions
                if (opcode == ZIROpcode::NOP)
                {
                    if (nopResultToInstruction.find(result) != nopResultToInstruction.end())
                    {
                        // Found redundant NOP
                        redundantPairs.push_back({nopResultToInstruction[result], instr});
                    }
                    else
                    {
                        // First time seeing this NOP result
                        nopResultToInstruction[result] = instr;
                    }
                }
                // Handle arithmetic instructions
                else if (opcode == ZIROpcode::ADD ||
                         opcode == ZIROpcode::SUB ||
                         opcode == ZIROpcode::MUL ||
                         opcode == ZIROpcode::DIV)
                {
                    auto binOp = std::dynamic_pointer_cast<BinaryArithmeticInst>(instr);
                    if (!binOp)
                        continue;

                    auto left = binOp->getLeft();
                    auto right = binOp->getRight();

                    if (!left || !right)
                        continue;

                    // Create expression string based on operands
                    std::string leftStr = left->toString();
                    std::string rightStr = right->toString();
                    std::string exprStr;

                    // For commutative operations, order operands consistently
                    if (opcode == ZIROpcode::ADD || opcode == ZIROpcode::MUL)
                    {
                        if (leftStr > rightStr)
                        {
                            exprStr = std::to_string(static_cast<int>(opcode)) + ":" + rightStr + ":" + leftStr;
                        }
                        else
                        {
                            exprStr = std::to_string(static_cast<int>(opcode)) + ":" + leftStr + ":" + rightStr;
                        }
                    }
                    else
                    {
                        // Non-commutative operations preserve order
                        exprStr = std::to_string(static_cast<int>(opcode)) + ":" + leftStr + ":" + rightStr;
                    }

                    // Check if we've seen this expression before
                    if (expressionToInstruction.find(exprStr) != expressionToInstruction.end())
                    {
                        // Found redundant expression
                        redundantPairs.push_back({expressionToInstruction[exprStr], instr});
                    }
                    else
                    {
                        // First time seeing this expression
                        expressionToInstruction[exprStr] = instr;
                    }
                }
            }
        }

        return redundantPairs;
    }

} // namespace zir