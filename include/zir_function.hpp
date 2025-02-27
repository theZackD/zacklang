#ifndef ZIR_FUNCTION_HPP
#define ZIR_FUNCTION_HPP

#include <string>
#include <vector>
#include <atomic>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include "zir_basic_block.hpp"

namespace zir
{
    class ZIRFunctionImpl : public std::enable_shared_from_this<ZIRFunctionImpl>
    {
    public:
        // Constructor and destructor
        explicit ZIRFunctionImpl(std::string name)
            : name(std::move(name)), id(next_id++) {}

        ~ZIRFunctionImpl() = default;

        // Prevent copying
        ZIRFunctionImpl(const ZIRFunctionImpl &) = delete;
        ZIRFunctionImpl &operator=(const ZIRFunctionImpl &) = delete;

        // Allow moving
        ZIRFunctionImpl(ZIRFunctionImpl &&) = default;
        ZIRFunctionImpl &operator=(ZIRFunctionImpl &&) = default;

        // Name management
        const std::string &getName() const { return name; }
        void setName(std::string new_name) { name = std::move(new_name); }

        // ID accessor
        uint64_t getId() const { return id; }

        // Block management
        void addBlock(std::shared_ptr<ZIRBasicBlockImpl> block);
        void removeBlock(std::shared_ptr<ZIRBasicBlockImpl> block);
        size_t getBlockCount() const { return blocks.size(); }
        std::shared_ptr<ZIRBasicBlockImpl> getBlock(size_t index) const;
        const std::vector<std::shared_ptr<ZIRBasicBlockImpl>> &getBlocks() const { return blocks; }

        // Dead block analysis and elimination
        std::vector<std::shared_ptr<ZIRBasicBlockImpl>> findDeadBlocks() const;
        size_t removeDeadBlocks();

        // Global value numbering
        std::unordered_map<std::string, size_t> performGlobalValueNumbering();
        std::vector<std::pair<std::shared_ptr<ZIRInstructionImpl>, std::shared_ptr<ZIRInstructionImpl>>> findGlobalRedundantComputations() const;
        bool hasGlobalRedundantComputations() const;

    private:
        std::string name;
        uint64_t id;
        std::vector<std::shared_ptr<ZIRBasicBlockImpl>> blocks;
        static std::atomic<uint64_t> next_id;
    };

} // namespace zir

#endif // ZIR_FUNCTION_HPP