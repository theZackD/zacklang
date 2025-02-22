#include "../include/zir_basic_block.hpp"
#include <queue>
#include <algorithm>

namespace zir
{
    std::atomic<uint64_t> ZIRBasicBlockImpl::next_id{0};

    // Check if this block is part of a cycle
    bool ZIRBasicBlockImpl::isInCycle() const
    {
        std::unordered_set<const ZIRBasicBlockImpl *> visited;
        std::unordered_set<const ZIRBasicBlockImpl *> recursionStack;
        std::vector<std::shared_ptr<ZIRBasicBlockImpl>> cycle;
        return detectCycleHelper(visited, recursionStack, cycle);
    }

    // Detect and return a cycle if one exists
    std::vector<std::shared_ptr<ZIRBasicBlockImpl>> ZIRBasicBlockImpl::detectCycle() const
    {
        std::unordered_set<const ZIRBasicBlockImpl *> visited;
        std::unordered_set<const ZIRBasicBlockImpl *> recursionStack;
        std::vector<std::shared_ptr<ZIRBasicBlockImpl>> cycle;
        if (detectCycleHelper(visited, recursionStack, cycle))
        {
            // Reverse the cycle to get correct order
            std::reverse(cycle.begin(), cycle.end());

            // Add the starting block to complete the cycle
            auto nonConstThis = const_cast<ZIRBasicBlockImpl *>(this);
            cycle.push_back(std::shared_ptr<ZIRBasicBlockImpl>(nonConstThis, [](ZIRBasicBlockImpl *) {}));
        }
        return cycle;
    }

    // Helper method for cycle detection using DFS
    bool ZIRBasicBlockImpl::detectCycleHelper(
        std::unordered_set<const ZIRBasicBlockImpl *> &visited,
        std::unordered_set<const ZIRBasicBlockImpl *> &recursionStack,
        std::vector<std::shared_ptr<ZIRBasicBlockImpl>> &cycle) const
    {
        visited.insert(this);
        recursionStack.insert(this);

        for (const auto &succ : successors)
        {
            if (!visited.count(succ.get()))
            {
                if (succ->detectCycleHelper(visited, recursionStack, cycle))
                {
                    cycle.push_back(succ);
                    return true;
                }
            }
            else if (recursionStack.count(succ.get()))
            {
                cycle.push_back(succ);
                return true;
            }
        }

        recursionStack.erase(this);
        return false;
    }

    // Check if this block can reach the target block
    bool ZIRBasicBlockImpl::canReach(const std::shared_ptr<ZIRBasicBlockImpl> &target) const
    {
        if (!target)
            return false;
        std::unordered_set<const ZIRBasicBlockImpl *> visited;
        return canReachHelper(target, visited);
    }

    bool ZIRBasicBlockImpl::isReachableFrom(const std::shared_ptr<ZIRBasicBlockImpl> &start) const
    {
        if (!start)
            return false;
        // Create a non-const shared pointer to this block
        auto nonConstThis = const_cast<ZIRBasicBlockImpl *>(this);
        auto thisPtr = std::shared_ptr<ZIRBasicBlockImpl>(nonConstThis, [](ZIRBasicBlockImpl *) {});
        return start->canReach(thisPtr);
    }

    // Helper method for reachability analysis using DFS
    bool ZIRBasicBlockImpl::canReachHelper(
        const std::shared_ptr<ZIRBasicBlockImpl> &target,
        std::unordered_set<const ZIRBasicBlockImpl *> &visited) const
    {
        if (this == target.get())
            return true;
        visited.insert(this);

        for (const auto &succ : successors)
        {
            if (!visited.count(succ.get()) && succ->canReachHelper(target, visited))
            {
                return true;
            }
        }

        return false;
    }

    // Get all blocks reachable from this block
    std::vector<std::shared_ptr<ZIRBasicBlockImpl>> ZIRBasicBlockImpl::getReachableBlocks() const
    {
        std::vector<std::shared_ptr<ZIRBasicBlockImpl>> reachable;
        std::unordered_set<const ZIRBasicBlockImpl *> visited;
        std::queue<std::shared_ptr<ZIRBasicBlockImpl>> queue;

        // Start with this block
        auto nonConstThis = const_cast<ZIRBasicBlockImpl *>(this);
        queue.push(std::shared_ptr<ZIRBasicBlockImpl>(nonConstThis, [](ZIRBasicBlockImpl *) {}));
        visited.insert(this);

        while (!queue.empty())
        {
            auto current = queue.front();
            queue.pop();
            reachable.push_back(current);

            for (const auto &succ : current->successors)
            {
                if (!visited.count(succ.get()))
                {
                    visited.insert(succ.get());
                    queue.push(succ);
                }
            }
        }

        return reachable;
    }

    // Compute dominators for the CFG
    void ZIRBasicBlockImpl::computeDominators(
        std::unordered_map<const ZIRBasicBlockImpl *, std::unordered_set<const ZIRBasicBlockImpl *>> &dominators) const
    {
        // Initialize all nodes to dominate themselves
        std::vector<std::shared_ptr<ZIRBasicBlockImpl>> allBlocks = getReachableBlocks();
        for (const auto &block : allBlocks)
        {
            dominators[block.get()].insert(block.get());
        }

        // Iteratively compute dominators
        bool changed;
        do
        {
            changed = false;
            for (const auto &block : allBlocks)
            {
                if (block.get() == this)
                    continue; // Skip entry block

                std::unordered_set<const ZIRBasicBlockImpl *> newDoms;
                bool first = true;

                // Intersect dominators of all predecessors
                for (const auto &pred : block->predecessors)
                {
                    if (first)
                    {
                        newDoms = dominators[pred.get()];
                        first = false;
                    }
                    else
                    {
                        std::unordered_set<const ZIRBasicBlockImpl *> intersection;
                        for (const auto &dom : newDoms)
                        {
                            if (dominators[pred.get()].count(dom))
                            {
                                intersection.insert(dom);
                            }
                        }
                        newDoms = intersection;
                    }
                }

                // Add the block itself
                newDoms.insert(block.get());

                // Check if dominators changed
                if (newDoms != dominators[block.get()])
                {
                    changed = true;
                    dominators[block.get()] = newDoms;
                }
            }
        } while (changed);
    }

    // Check if this block dominates another block
    bool ZIRBasicBlockImpl::dominates(const std::shared_ptr<ZIRBasicBlockImpl> &other) const
    {
        if (!other)
            return false;
        std::unordered_map<const ZIRBasicBlockImpl *, std::unordered_set<const ZIRBasicBlockImpl *>> dominators;
        computeDominators(dominators);
        return dominators[other.get()].count(this);
    }

    // Check if this block post-dominates another block
    bool ZIRBasicBlockImpl::postDominates(const std::shared_ptr<ZIRBasicBlockImpl> &other) const
    {
        if (!other)
            return false;

        // Create a reversed graph where each block points to its predecessors
        std::unordered_map<const ZIRBasicBlockImpl *, std::shared_ptr<ZIRBasicBlockImpl>> reversedBlocks;
        std::queue<const ZIRBasicBlockImpl *> queue;
        queue.push(other.get());

        // First, create all reversed blocks
        while (!queue.empty())
        {
            auto current = queue.front();
            queue.pop();

            if (reversedBlocks.find(current) == reversedBlocks.end())
            {
                reversedBlocks[current] = std::make_shared<ZIRBasicBlockImpl>(current->getName());

                // Add unprocessed successors to queue
                for (const auto &succ : current->successors)
                {
                    if (reversedBlocks.find(succ.get()) == reversedBlocks.end())
                    {
                        queue.push(succ.get());
                    }
                }
            }
        }

        // Now connect the reversed blocks
        for (const auto &[block, reversedBlock] : reversedBlocks)
        {
            for (const auto &succ : block->successors)
            {
                if (reversedBlocks.find(succ.get()) != reversedBlocks.end())
                {
                    reversedBlocks[succ.get()]->addSuccessor(reversedBlock);
                }
            }
        }

        // Check if the reversed block of 'this' dominates the reversed block of 'other'
        auto reversedThis = reversedBlocks[this];
        auto reversedOther = reversedBlocks[other.get()];

        if (!reversedThis || !reversedOther)
            return false;

        std::unordered_map<const ZIRBasicBlockImpl *, std::unordered_set<const ZIRBasicBlockImpl *>> dominators;
        reversedThis->computeDominators(dominators);

        return dominators[reversedOther.get()].count(reversedThis.get());
    }

    // Get the dominance frontier for this block
    std::vector<std::shared_ptr<ZIRBasicBlockImpl>> ZIRBasicBlockImpl::getDominanceFrontier() const
    {
        std::vector<std::shared_ptr<ZIRBasicBlockImpl>> frontier;
        std::unordered_map<const ZIRBasicBlockImpl *, std::unordered_set<const ZIRBasicBlockImpl *>> dominators;
        computeDominators(dominators);

        // For each block we dominate
        for (const auto &[block, doms] : dominators)
        {
            if (doms.count(this))
            {
                // Check its successors
                auto blockPtr = std::shared_ptr<ZIRBasicBlockImpl>(const_cast<ZIRBasicBlockImpl *>(block), [](ZIRBasicBlockImpl *) {});
                for (const auto &succ : blockPtr->successors)
                {
                    // If we don't strictly dominate the successor, it's in our frontier
                    if (!dominators[succ.get()].count(this))
                    {
                        frontier.push_back(succ);
                    }
                }
            }
        }

        return frontier;
    }
}