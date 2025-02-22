#include "../include/zir_function.hpp"
#include "../include/zir_c_api.h"
#include <stdexcept>
#include <algorithm>
#include <iostream>

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

} // namespace zir