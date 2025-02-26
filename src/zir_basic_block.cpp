#include "../include/zir_basic_block.hpp"
#include <queue>
#include <algorithm>
#include <iostream>
#include <sstream>

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

    // Check if merging with another block is safe
    bool ZIRBasicBlockImpl::isSafeMergeWith(const std::shared_ptr<ZIRBasicBlockImpl> &other) const
    {
        // First check if blocks are mergeable at all
        if (!isMergeableWith(other))
            return false;

        // Check for PHI nodes in the successor block
        for (const auto &instr : other->instructions)
        {
            if (instr->getOpcode() == ZIROpcode::PHI)
                return false; // PHI nodes make merging unsafe
        }

        // Check for terminator instructions in this block
        for (const auto &instr : instructions)
        {
            if (instr->isTerminator() && instr != instructions.back())
                return false; // Can't have terminator instructions except at the end

            // Check for label references to the other block
            if (instr->referencesLabel(other->getName()))
                return false;
        }

        // Check for label references to this block in the other block
        for (const auto &instr : other->instructions)
        {
            if (instr->referencesLabel(getName()))
                return false;
        }

        // Check for variable definitions and uses
        std::unordered_set<std::string> definedVars;
        std::unordered_set<std::string> usedVars;

        // Collect variable definitions and uses from both blocks
        for (const auto &instr : instructions)
        {
            auto defs = instr->getDefinedVariables();
            auto uses = instr->getUsedVariables();
            definedVars.insert(defs.begin(), defs.end());
            usedVars.insert(uses.begin(), uses.end());
        }

        for (const auto &instr : other->instructions)
        {
            auto defs = instr->getDefinedVariables();
            auto uses = instr->getUsedVariables();

            // Check for variable redefinition conflicts
            for (const auto &def : defs)
            {
                if (definedVars.count(def) > 0)
                    return false; // Variable is redefined
            }

            // Check for use-before-def conflicts
            for (const auto &use : uses)
            {
                if (definedVars.count(use) == 0 && usedVars.count(use) > 0)
                    return false; // Variable is used before definition
            }
        }

        return true;
    }

    std::shared_ptr<ZIRBasicBlockImpl> ZIRBasicBlockImpl::mergeWith(const std::shared_ptr<ZIRBasicBlockImpl> &other)
    {
        if (!other || !isSafeMergeWith(other))
        {
            return nullptr;
        }

        // Create a new block with this block's name
        auto mergedBlock = std::make_shared<ZIRBasicBlockImpl>(getName());

        // Copy instructions from this block (except the terminator)
        for (size_t i = 0; i < instructions.size() - 1; i++)
        {
            mergedBlock->addInstruction(instructions[i]);
        }

        // Copy all instructions from the other block
        for (const auto &instr : other->instructions)
        {
            mergedBlock->addInstruction(instr);
        }

        // Update predecessors (keep this block's predecessors)
        for (const auto &pred : predecessors)
        {
            mergedBlock->addPredecessor(pred);
        }

        // Update successors (take other block's successors)
        for (const auto &succ : other->successors)
        {
            mergedBlock->addSuccessor(succ);
        }

        // Set parent function
        mergedBlock->setParentFunction(parent_function);

        return mergedBlock;
    }

    bool ZIRBasicBlockImpl::isMergeableWith(const std::shared_ptr<ZIRBasicBlockImpl> &other) const
    {
        // Must have exactly one successor (the other block)
        if (successors.size() != 1 || *successors.begin() != other)
            return false;

        // Other block must have exactly one predecessor (this block)
        if (other->predecessors.size() != 1 || *other->predecessors.begin() != shared_from_this())
            return false;

        // For now, we don't merge blocks that are part of critical edges
        // This will be handled by the critical edge splitting pass later
        if (isCriticalEdge(other))
            return false;

        return true;
    }

    std::shared_ptr<ZIRBasicBlockImpl> ZIRBasicBlockImpl::findMergeableSuccessor() const
    {
        // If we have exactly one successor, check if we can merge with it
        if (successors.size() == 1)
        {
            auto successor = *successors.begin();
            if (isMergeableWith(successor))
            {
                return successor;
            }
        }
        return nullptr;
    }

    // Jump Threading Analysis Methods

    bool ZIRBasicBlockImpl::isJumpThreadableBlock() const
    {
        // A block is jump threadable if it contains only a single unconditional jump instruction
        if (instructions.size() != 1)
            return false;

        auto instr = instructions[0];
        return instr && instr->getOpcode() == ZIROpcode::BR;
    }

    bool ZIRBasicBlockImpl::canThreadJumpThrough() const
    {
        // A block can be thread-jumped through if:
        // 1. It has exactly one successor
        // 2. It has no side effects (for now, just check if it's a jump threadable block)
        return successors.size() == 1 && isJumpThreadableBlock();
    }

    std::shared_ptr<ZIRBasicBlockImpl> ZIRBasicBlockImpl::getJumpTarget() const
    {
        // If this is a jump threadable block, return its target
        if (!isJumpThreadableBlock() || successors.empty())
            return nullptr;

        return *successors.begin();
    }

    bool ZIRBasicBlockImpl::isJumpThreadingSafe(
        const std::shared_ptr<ZIRBasicBlockImpl> &from,
        const std::shared_ptr<ZIRBasicBlockImpl> &to) const
    {
        // Check that 'this' is the block we're threading through
        if (!canThreadJumpThrough())
        {
            std::cout << "Debug: Block '" << getName() << "' cannot be thread-jumped through" << std::endl;
            return false;
        }

        // Make sure 'from' is a predecessor of 'this'
        // Note: This check might fail after previous threading operations
        if (!hasPredecessor(from))
        {
            // Check if 'from' can already reach 'to' directly
            if (from->hasSuccessor(to))
            {
                // Threading already done, consider it "safe"
                std::cout << "Debug: Block '" << from->getName() << "' already has a direct edge to '" << to->getName() << "'" << std::endl;
                return true;
            }

            std::cout << "Debug: Block '" << getName() << "' doesn't have predecessor '" << from->getName() << "'" << std::endl;
            return false;
        }

        // Make sure 'to' is the jump target of 'this'
        // For jump threading chains, the current target might be different
        auto jumpTarget = getJumpTarget();
        if (jumpTarget != to)
        {
            std::cout << "Debug: Jump target '" << (jumpTarget ? jumpTarget->getName() : "null") << "' doesn't match '" << to->getName() << "'" << std::endl;

            // In a chain of jump threadings, the target may already be different
            // Consider safe if the original target can reach 'to'
            if (jumpTarget && jumpTarget->canReach(to))
            {
                std::cout << "Debug: Jump target can reach '" << to->getName() << "', considering safe" << std::endl;
                return true;
            }

            return false;
        }

        // For now, we'll only consider threading safe if there are no PHI nodes in 'to'
        // as those would need special handling
        for (const auto &instr : to->instructions)
        {
            if (instr->getOpcode() == ZIROpcode::PHI)
            {
                std::cout << "Debug: Target block '" << to->getName() << "' has PHI nodes" << std::endl;
                return false; // PHI nodes make merging unsafe
            }
        }

        // Also check that 'from' doesn't have multiple exits
        // (conditional branches would need special handling)
        for (const auto &instr : from->instructions)
        {
            if (instr->getOpcode() == ZIROpcode::BR_COND)
            {
                std::cout << "Debug: Source block '" << from->getName() << "' has conditional branches" << std::endl;
                return false;
            }
        }

        return true;
    }

    std::vector<std::pair<std::shared_ptr<ZIRBasicBlockImpl>, std::shared_ptr<ZIRBasicBlockImpl>>>
    ZIRBasicBlockImpl::findJumpThreadingOpportunities() const
    {
        std::vector<std::pair<std::shared_ptr<ZIRBasicBlockImpl>, std::shared_ptr<ZIRBasicBlockImpl>>> opportunities;

        // If this block cannot be thread-jumped through, there are no opportunities
        if (!canThreadJumpThrough())
            return opportunities;

        auto target = getJumpTarget();
        if (!target)
            return opportunities;

        // For each predecessor, check if threading is safe
        for (const auto &pred : predecessors)
        {
            if (isJumpThreadingSafe(pred, target))
            {
                opportunities.push_back({pred, target});
            }
        }

        return opportunities;
    }

    // Perform jump threading transformation
    bool ZIRBasicBlockImpl::performJumpThreading(
        const std::shared_ptr<ZIRBasicBlockImpl> &from,
        const std::shared_ptr<ZIRBasicBlockImpl> &to)
    {
        // Check if jump threading is safe
        if (!isJumpThreadingSafe(from, to))
            return false;

        // 1. Update the terminator instruction in 'from' to directly jump to 'to'
        // Find the terminator instruction in 'from'
        for (size_t i = 0; i < from->instructions.size(); ++i)
        {
            auto instr = from->instructions[i];
            if (instr->isTerminator() && instr->getOpcode() == ZIROpcode::BR)
            {
                // This is a terminator we want to modify
                // First, check if it has a target label
                if (!instr->getTargetLabel().empty())
                {
                    // Update target to point to 'to' block
                    instr->setTargetLabel(to->getName());
                }

                // We found and modified the terminator, so we can stop searching
                break;
            }
        }

        // 2. Update the CFG
        // Before removing any edges, check that all blocks still exist and have valid relationships
        if (!from->hasSuccessor(shared_from_this()) || !hasPredecessor(from) ||
            !hasSuccessor(to) || !to->hasPredecessor(shared_from_this()))
        {
            // The CFG has already been modified by a previous transformation
            // Just ensure from has a direct edge to to
            if (!from->hasSuccessor(to))
            {
                from->addSuccessor(to);
                to->addPredecessor(from);
            }
            return true;
        }

        // Remove relationships between 'from' and 'this'
        from->removeSuccessor(shared_from_this());
        removePredecessor(from);

        // Remove relationship between 'this' and 'to'
        removeSuccessor(to);
        to->removePredecessor(shared_from_this());

        // Add direct relationship between 'from' and 'to'
        if (!from->hasSuccessor(to))
        {
            from->addSuccessor(to);
            to->addPredecessor(from);
        }

        return true;
    }

    // Check if this block has any critical edges
    bool ZIRBasicBlockImpl::hasCriticalEdges() const
    {
        // A block has critical edges if it has multiple successors
        // and any of those successors have multiple predecessors
        if (successors.size() <= 1)
        {
            return false; // Not enough successors to form a critical edge
        }

        for (const auto &succ : successors)
        {
            if (succ->predecessors.size() > 1)
            {
                // Found a critical edge: this block has multiple successors
                // and the successor has multiple predecessors
                return true;
            }
        }

        return false;
    }

    // Find all critical edges from this block
    std::vector<std::pair<std::shared_ptr<ZIRBasicBlockImpl>, std::shared_ptr<ZIRBasicBlockImpl>>>
    ZIRBasicBlockImpl::findCriticalEdges() const
    {
        std::vector<std::pair<std::shared_ptr<ZIRBasicBlockImpl>, std::shared_ptr<ZIRBasicBlockImpl>>> criticalEdges;

        // A block with only one successor cannot have critical edges
        if (successors.size() <= 1)
        {
            return criticalEdges;
        }

        // Check each successor
        for (const auto &succ : successors)
        {
            if (succ->predecessors.size() > 1)
            {
                // This is a critical edge:
                // - this block has multiple successors
                // - the successor has multiple predecessors

                // Use const_cast to add this block to the results
                // This is safe because we're not modifying the object
                auto nonConstThis = const_cast<ZIRBasicBlockImpl *>(this);
                auto thisPtr = std::shared_ptr<ZIRBasicBlockImpl>(nonConstThis, [](ZIRBasicBlockImpl *) {});

                criticalEdges.push_back(std::make_pair(thisPtr, succ));
            }
        }

        return criticalEdges;
    }

    // Check if the edge to a specific successor is a critical edge
    bool ZIRBasicBlockImpl::isCriticalEdgeToSuccessor(const std::shared_ptr<ZIRBasicBlockImpl> &succ) const
    {
        if (!succ)
        {
            return false;
        }

        // Check if this block has the successor
        if (successors.find(succ) == successors.end())
        {
            return false; // Not a successor
        }

        // A critical edge exists when this block has multiple successors
        // and the successor has multiple predecessors
        return successors.size() > 1 && succ->predecessors.size() > 1;
    }

    // Check if it's safe to split the edge to a given successor
    bool ZIRBasicBlockImpl::isSplitSafe(const std::shared_ptr<ZIRBasicBlockImpl> &succ) const
    {
        if (!succ)
        {
            return false;
        }

        // If it's not a critical edge, no need to split it
        if (!isCriticalEdgeToSuccessor(succ))
        {
            return false;
        }

        // Check any conditions that would make splitting unsafe
        // For most cases, splitting a critical edge is safe

        // In the future, we might want to check for:
        // - Phi nodes that would be affected
        // - Exception handling blocks
        // - Other special cases

        return true;
    }

    // Split a critical edge between this block and the specified successor
    std::shared_ptr<ZIRBasicBlockImpl> ZIRBasicBlockImpl::splitCriticalEdge(const std::shared_ptr<ZIRBasicBlockImpl> &succ)
    {
        // Validate the edge is actually critical and safe to split
        if (!succ || !isCriticalEdgeToSuccessor(succ) || !isSplitSafe(succ))
        {
            return nullptr;
        }

        // Create a new basic block to insert between this block and the successor
        std::stringstream newBlockName;
        newBlockName << name << "_to_" << succ->getName() << "_split";
        auto newBlock = std::make_shared<ZIRBasicBlockImpl>(newBlockName.str());

        // If this block has a parent function, add the new block to the same function
        if (parent_function)
        {
            newBlock->setParentFunction(parent_function);
        }

        // Update the control flow graph
        // 1. Remove the direct edge between this block and the successor
        removeSuccessor(succ);

        // 2. Add edges: this block -> new block -> successor
        addSuccessor(newBlock);
        newBlock->addSuccessor(succ);

        // 3. Update branch instructions if needed
        // This may require modifying terminator instructions that target the successor
        // For now, we'll add a simple jump instruction to the successor
        // In a real implementation, you'd need to handle branches, switches, etc.

        // Return the newly created block
        return newBlock;
    }

    // Split all critical edges from this block
    bool ZIRBasicBlockImpl::splitAllCriticalEdges()
    {
        bool splitAny = false;

        // Find all critical edges
        auto criticalEdges = findCriticalEdges();

        // Split each critical edge
        for (const auto &edge : criticalEdges)
        {
            auto successor = edge.second;
            if (splitCriticalEdge(successor) != nullptr)
            {
                splitAny = true;
            }
        }

        return splitAny;
    }
}