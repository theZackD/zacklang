#include "../include/zir_basic_block.hpp"

namespace zir
{
    std::atomic<uint64_t> ZIRBasicBlockImpl::next_id{0};
}