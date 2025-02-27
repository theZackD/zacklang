#ifndef ZIR_BASIC_BLOCK_HPP
#define ZIR_BASIC_BLOCK_HPP

#include <string>
#include <memory>
#include <vector>
#include <atomic>
#include <set>
#include <stdexcept>
#include "zir_value.hpp"
#include "zir_instruction.hpp"
#include <unordered_set>
#include <unordered_map>
#include <iostream>

namespace zir
{

    // Forward declaration for function
    class ZIRFunctionImpl;

    class ZIRBasicBlockImpl : public std::enable_shared_from_this<ZIRBasicBlockImpl>
    {
    public:
        // Constructor takes a name for the block and optional parent function
        explicit ZIRBasicBlockImpl(std::string name)
            : name(std::move(name)), id(next_id++), parent_function(nullptr) {}

        // Destructor
        ~ZIRBasicBlockImpl() = default;

        // Prevent copying
        ZIRBasicBlockImpl(const ZIRBasicBlockImpl &) = delete;
        ZIRBasicBlockImpl &operator=(const ZIRBasicBlockImpl &) = delete;

        // Allow moving
        ZIRBasicBlockImpl(ZIRBasicBlockImpl &&) = default;
        ZIRBasicBlockImpl &operator=(ZIRBasicBlockImpl &&) = default;

        // Get/set parent function (as opaque handle)
        void *getParentFunction() const
        {
            std::cout << "Debug: Getting parent function: " << parent_function << std::endl;
            return parent_function;
        }
        void setParentFunction(void *parent)
        {
            std::cout << "Debug: Setting parent function to " << parent << std::endl;
            parent_function = parent;
        }

        // Get/set name
        const std::string &getName() const { return name; }
        void setName(std::string new_name) { name = std::move(new_name); }

        // Get unique ID
        uint64_t getId() const { return id; }

        // Instruction management
        void addInstruction(std::shared_ptr<ZIRInstructionImpl> instruction)
        {
            if (!instruction)
                return;
            instructions.push_back(instruction);
        }

        void removeInstruction(size_t index)
        {
            if (index < instructions.size())
            {
                instructions.erase(instructions.begin() + index);
            }
        }

        std::shared_ptr<ZIRInstructionImpl> getInstruction(size_t index) const
        {
            if (index < instructions.size())
            {
                return instructions[index];
            }
            return nullptr;
        }

        size_t getInstructionCount() const
        {
            return instructions.size();
        }

        const std::vector<std::shared_ptr<ZIRInstructionImpl>> &getInstructions() const
        {
            return instructions;
        }

        // Block linking
        void addPredecessor(std::shared_ptr<ZIRBasicBlockImpl> pred)
        {
            if (!pred)
                throw std::invalid_argument("Cannot add null predecessor");
            if (predecessors.insert(pred).second)
            {
                // Only add the successor link if the predecessor was newly added
                pred->addSuccessorNoRecurse(shared_from_this());
            }
        }

        void addSuccessor(std::shared_ptr<ZIRBasicBlockImpl> succ)
        {
            if (!succ)
                throw std::invalid_argument("Cannot add null successor");
            if (successors.insert(succ).second)
            {
                // Only add the predecessor link if the successor was newly added
                succ->addPredecessorNoRecurse(shared_from_this());
            }
        }

        void removePredecessor(std::shared_ptr<ZIRBasicBlockImpl> pred)
        {
            if (!pred)
                return;
            if (predecessors.erase(pred) > 0)
            {
                // Only remove the successor link if the predecessor was actually removed
                pred->removeSuccessorNoRecurse(shared_from_this());
            }
        }

        void removeSuccessor(std::shared_ptr<ZIRBasicBlockImpl> succ)
        {
            if (!succ)
                return;
            if (successors.erase(succ) > 0)
            {
                // Only remove the predecessor link if the successor was actually removed
                succ->removePredecessorNoRecurse(shared_from_this());
            }
        }

        // Graph query methods
        const std::set<std::shared_ptr<ZIRBasicBlockImpl>> &getPredecessors() const
        {
            return predecessors;
        }

        const std::set<std::shared_ptr<ZIRBasicBlockImpl>> &getSuccessors() const
        {
            return successors;
        }

        size_t getPredecessorCount() const
        {
            return predecessors.size();
        }

        size_t getSuccessorCount() const
        {
            return successors.size();
        }

        bool hasSuccessor(std::shared_ptr<ZIRBasicBlockImpl> block) const
        {
            return successors.find(block) != successors.end();
        }

        bool hasPredecessor(std::shared_ptr<ZIRBasicBlockImpl> block) const
        {
            return predecessors.find(block) != predecessors.end();
        }

        // Graph analysis methods
        bool isInCycle() const;
        std::vector<std::shared_ptr<ZIRBasicBlockImpl>> detectCycle() const;
        bool canReach(const std::shared_ptr<ZIRBasicBlockImpl> &target) const;
        std::vector<std::shared_ptr<ZIRBasicBlockImpl>> getReachableBlocks() const;
        bool dominates(const std::shared_ptr<ZIRBasicBlockImpl> &other) const;
        bool postDominates(const std::shared_ptr<ZIRBasicBlockImpl> &other) const;
        std::vector<std::shared_ptr<ZIRBasicBlockImpl>> getDominanceFrontier() const;

        // Block merging
        bool isMergeableWith(const std::shared_ptr<ZIRBasicBlockImpl> &other) const;
        bool isSafeMergeWith(const std::shared_ptr<ZIRBasicBlockImpl> &other) const;
        std::shared_ptr<ZIRBasicBlockImpl> mergeWith(const std::shared_ptr<ZIRBasicBlockImpl> &other);
        std::shared_ptr<ZIRBasicBlockImpl> findMergeableSuccessor() const;

        // Jump threading
        bool isJumpThreadableBlock() const;
        bool canThreadJumpThrough() const;
        std::shared_ptr<ZIRBasicBlockImpl> getJumpTarget() const;
        bool isJumpThreadingSafe(const std::shared_ptr<ZIRBasicBlockImpl> &from,
                                 const std::shared_ptr<ZIRBasicBlockImpl> &to) const;
        std::vector<std::pair<std::shared_ptr<ZIRBasicBlockImpl>, std::shared_ptr<ZIRBasicBlockImpl>>>
        findJumpThreadingOpportunities() const;

        // Perform jump threading transformation
        bool performJumpThreading(const std::shared_ptr<ZIRBasicBlockImpl> &from,
                                  const std::shared_ptr<ZIRBasicBlockImpl> &to);

        // Critical edge detection
        bool hasCriticalEdges() const;
        std::vector<std::pair<std::shared_ptr<ZIRBasicBlockImpl>, std::shared_ptr<ZIRBasicBlockImpl>>>
        findCriticalEdges() const;
        bool isCriticalEdgeToSuccessor(const std::shared_ptr<ZIRBasicBlockImpl> &succ) const;
        bool isSplitSafe(const std::shared_ptr<ZIRBasicBlockImpl> &succ) const;

        // Critical edge splitting
        std::shared_ptr<ZIRBasicBlockImpl> splitCriticalEdge(const std::shared_ptr<ZIRBasicBlockImpl> &succ);
        bool splitAllCriticalEdges();

        // Value numbering
        struct ValueNumber
        {
            size_t number;
            std::string expression;
            ZIROpcode opcode;
            std::vector<size_t> operands;

            bool operator==(const ValueNumber &other) const
            {
                return opcode == other.opcode && operands == other.operands;
            }
        };

        // Local value numbering
        std::unordered_map<std::string, size_t> performLocalValueNumbering();
        bool hasRedundantComputations() const;
        std::vector<std::pair<size_t, size_t>> findRedundantComputations() const;

        // Reachability analysis
        bool isReachableFrom(const std::shared_ptr<ZIRBasicBlockImpl> &start) const
        {
            if (!start)
                return false;
            if (start.get() == this)
                return true;

            std::unordered_set<const ZIRBasicBlockImpl *> visited;
            return isReachableFromHelper(start.get(), visited);
        }

    private:
        std::string name;
        uint64_t id;
        static std::atomic<uint64_t> next_id;
        void *parent_function; // Store as void* to avoid circular dependency
        std::vector<std::shared_ptr<ZIRInstructionImpl>> instructions;
        std::set<std::shared_ptr<ZIRBasicBlockImpl>> predecessors;
        std::set<std::shared_ptr<ZIRBasicBlockImpl>> successors;

        // Helper methods for maintaining bidirectional links
        void addPredecessorNoRecurse(std::shared_ptr<ZIRBasicBlockImpl> pred)
        {
            predecessors.insert(pred);
        }

        void addSuccessorNoRecurse(std::shared_ptr<ZIRBasicBlockImpl> succ)
        {
            successors.insert(succ);
        }

        void removePredecessorNoRecurse(std::shared_ptr<ZIRBasicBlockImpl> pred)
        {
            predecessors.erase(pred);
        }

        void removeSuccessorNoRecurse(std::shared_ptr<ZIRBasicBlockImpl> succ)
        {
            successors.erase(succ);
        }

        bool isReachableFromHelper(const ZIRBasicBlockImpl *current,
                                   std::unordered_set<const ZIRBasicBlockImpl *> &visited) const
        {
            if (current == this)
                return true;
            if (!current || !visited.insert(current).second)
                return false;

            for (const auto &succ : current->successors)
            {
                if (isReachableFromHelper(succ.get(), visited))
                    return true;
            }
            return false;
        }

        // Helper methods for graph analysis
        bool detectCycleHelper(std::unordered_set<const ZIRBasicBlockImpl *> &visited,
                               std::unordered_set<const ZIRBasicBlockImpl *> &recursionStack,
                               std::vector<std::shared_ptr<ZIRBasicBlockImpl>> &cycle) const;
        bool canReachHelper(const std::shared_ptr<ZIRBasicBlockImpl> &target,
                            std::unordered_set<const ZIRBasicBlockImpl *> &visited) const;
        void computeDominators(std::unordered_map<const ZIRBasicBlockImpl *, std::unordered_set<const ZIRBasicBlockImpl *>> &dominators) const;

        // Check if the edge to the given successor is a critical edge
        bool isCriticalEdge(const std::shared_ptr<ZIRBasicBlockImpl> &succ) const
        {
            // A critical edge is an edge from a block with multiple successors
            // to a block with multiple predecessors
            return successors.size() > 1 && succ->predecessors.size() > 1;
        }
    };

} // namespace zir

#endif // ZIR_BASIC_BLOCK_HPP