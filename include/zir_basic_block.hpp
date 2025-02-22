#ifndef ZIR_BASIC_BLOCK_HPP
#define ZIR_BASIC_BLOCK_HPP

#include <string>
#include <memory>
#include <vector>
#include <atomic>
#include "zir_value.hpp"

namespace zir
{

    // Forward declaration for function
    class ZIRFunctionImpl;

    class ZIRBasicBlockImpl
    {
    public:
        // Constructor takes a name for the block and optional parent function
        explicit ZIRBasicBlockImpl(std::string name, void *parent = nullptr)
            : name(std::move(name)), parent_function_handle(parent), id(next_id++) {}

        // Destructor
        ~ZIRBasicBlockImpl() = default;

        // Prevent copying
        ZIRBasicBlockImpl(const ZIRBasicBlockImpl &) = delete;
        ZIRBasicBlockImpl &operator=(const ZIRBasicBlockImpl &) = delete;

        // Allow moving
        ZIRBasicBlockImpl(ZIRBasicBlockImpl &&) = default;
        ZIRBasicBlockImpl &operator=(ZIRBasicBlockImpl &&) = default;

        // Basic accessors
        const std::string &getName() const { return name; }
        void setName(std::string new_name) { name = std::move(new_name); }

        // ID accessor
        uint64_t getId() const { return id; }

        // Parent function accessors
        void *getParentFunction() const { return parent_function_handle; }
        void setParentFunction(void *parent) { parent_function_handle = parent; }

    private:
        std::string name;
        void *parent_function_handle;
        uint64_t id;
        static std::atomic<uint64_t> next_id;
        // We'll add instruction list and other features later
    };

} // namespace zir

#endif // ZIR_BASIC_BLOCK_HPP