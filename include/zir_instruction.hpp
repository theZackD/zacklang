#ifndef ZIR_INSTRUCTION_HPP
#define ZIR_INSTRUCTION_HPP

#include <string>
#include <memory>
#include "zir_type.hpp"
#include "zir_value.hpp"

namespace zir
{

    class ZIRInstructionImpl
    {
    public:
        // Constructor and virtual destructor
        explicit ZIRInstructionImpl(std::string name) : name(std::move(name)) {}
        virtual ~ZIRInstructionImpl() = default;

        // Basic accessors
        const std::string &getName() const { return name; }
        void setName(std::string new_name) { name = std::move(new_name); }

        // Pure virtual methods that all instructions must implement
        virtual std::string toString() const = 0;
        virtual ZIRType::Kind getResultType() const = 0;

        // Prevent copying
        ZIRInstructionImpl(const ZIRInstructionImpl &) = delete;
        ZIRInstructionImpl &operator=(const ZIRInstructionImpl &) = delete;

        // Allow moving
        ZIRInstructionImpl(ZIRInstructionImpl &&) = default;
        ZIRInstructionImpl &operator=(ZIRInstructionImpl &&) = default;

    private:
        std::string name;
    };

} // namespace zir

#endif // ZIR_INSTRUCTION_HPP