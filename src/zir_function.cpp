#include "../include/zir_function.hpp"
#include <stdexcept>
#include <algorithm>

namespace zir
{
    std::atomic<uint64_t> ZIRFunctionImpl::next_id{0};

    void ZIRFunctionImpl::addBlock(std::shared_ptr<ZIRBasicBlockImpl> block)
    {
        if (!block)
        {
            throw std::invalid_argument("Cannot add null block to function");
        }

        // Remove from previous parent if any
        if (auto parent = block->getParentFunction())
        {
            if (parent != this)
            {
                auto *parent_ptr = reinterpret_cast<std::shared_ptr<ZIRFunctionImpl> *>(parent);
                (*parent_ptr)->removeBlock(block);
            }
        }

        // Add the block to our list
        blocks.push_back(block);
    }

    void ZIRFunctionImpl::removeBlock(std::shared_ptr<ZIRBasicBlockImpl> block)
    {
        if (!block)
        {
            return;
        }

        auto it = std::find(blocks.begin(), blocks.end(), block);
        if (it != blocks.end())
        {
            // Only clear parent if we're the current parent
            if ((*it)->getParentFunction() == this)
            {
                (*it)->setParentFunction(nullptr);
            }
            blocks.erase(it);
        }
    }

    std::shared_ptr<ZIRBasicBlockImpl> ZIRFunctionImpl::getBlock(size_t index) const
    {
        if (index >= blocks.size())
        {
            return nullptr;
        }
        return blocks[index];
    }

} // namespace zir