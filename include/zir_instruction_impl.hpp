#ifndef ZIR_INSTRUCTION_IMPL_HPP
#define ZIR_INSTRUCTION_IMPL_HPP

#include "zir_instruction.hpp"

namespace zir
{

    // Basic instruction implementation for testing
    class BasicInstruction : public ZIRInstructionImpl
    {
    public:
        explicit BasicInstruction(const std::string &name)
            : ZIRInstructionImpl(name) {}

        std::string toString() const override
        {
            return getName();
        }

        ZIRType::Kind getResultType() const override
        {
            return ZIRType::Kind::Integer; // Default to integer for now
        }
    };

} // namespace zir

#endif // ZIR_INSTRUCTION_IMPL_HPP