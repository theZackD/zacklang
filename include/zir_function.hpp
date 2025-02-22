#ifndef ZIR_FUNCTION_HPP
#define ZIR_FUNCTION_HPP

#include <string>
#include <memory>
#include <vector>
#include <atomic>
#include "zir_basic_block.hpp"
#include <iostream>

namespace zir
{
    class ZIRFunctionImpl : public std::enable_shared_from_this<ZIRFunctionImpl>
    {
    public:
        // Constructor takes a name for the function
        explicit ZIRFunctionImpl(std::string name)
            : name(std::move(name)), id(next_id++) {}

        // Destructor
        ~ZIRFunctionImpl() = default;

        // Prevent copying
        ZIRFunctionImpl(const ZIRFunctionImpl &) = delete;
        ZIRFunctionImpl &operator=(const ZIRFunctionImpl &) = delete;

        // Allow moving
        ZIRFunctionImpl(ZIRFunctionImpl &&) = default;
        ZIRFunctionImpl &operator=(ZIRFunctionImpl &&) = default;

        // Basic accessors
        const std::string &getName() const { return name; }
        void setName(std::string new_name) { name = std::move(new_name); }

        // ID accessor
        uint64_t getId() const { return id; }

        // Block management
        void addBlock(std::shared_ptr<ZIRBasicBlockImpl> block)
        {
            if (!block)
                return;
            std::cout << "Debug: Adding block to function " << name << std::endl;
            blocks.push_back(block);
        }

        void removeBlock(std::shared_ptr<ZIRBasicBlockImpl> block)
        {
            if (!block)
                return;
            auto it = std::find(blocks.begin(), blocks.end(), block);
            if (it != blocks.end())
            {
                blocks.erase(it);
            }
        }

        size_t getBlockCount() const { return blocks.size(); }

        std::shared_ptr<ZIRBasicBlockImpl> getBlock(size_t index) const
        {
            if (index >= blocks.size())
                return nullptr;
            return blocks[index];
        }

        const std::vector<std::shared_ptr<ZIRBasicBlockImpl>> &getBlocks() const { return blocks; }

        // Dead block analysis and elimination
        std::vector<std::shared_ptr<ZIRBasicBlockImpl>> findDeadBlocks() const
        {
            std::vector<std::shared_ptr<ZIRBasicBlockImpl>> dead_blocks;
            if (blocks.empty())
                return dead_blocks;

            // First block is always entry point
            auto entry = blocks[0];

            // Check each block's reachability from entry
            for (const auto &block : blocks)
            {
                if (!block->isReachableFrom(entry))
                {
                    dead_blocks.push_back(block);
                }
            }
            return dead_blocks;
        }

        size_t removeDeadBlocks()
        {
            auto dead = findDeadBlocks();
            for (const auto &block : dead)
            {
                removeBlock(block);
            }
            return dead.size();
        }

    private:
        std::string name;
        uint64_t id;
        std::vector<std::shared_ptr<ZIRBasicBlockImpl>> blocks;
        static std::atomic<uint64_t> next_id;
    };

} // namespace zir

#endif // ZIR_FUNCTION_HPP