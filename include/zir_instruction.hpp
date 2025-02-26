#ifndef ZIR_INSTRUCTION_HPP
#define ZIR_INSTRUCTION_HPP

#include <string>
#include <memory>
#include <vector>
#include <unordered_set>
#include "zir_type.hpp"
#include "zir_value.hpp"

namespace zir
{

    // Define instruction opcodes
    enum class ZIROpcode
    {
        NOP,
        ADD,
        SUB,
        MUL,
        DIV,
        LOAD,
        STORE,
        BR,
        BR_COND,
        PHI,
        RET
    };

    class ZIRInstructionImpl
    {
    public:
        // Constructors
        explicit ZIRInstructionImpl(const std::string &name)
            : opcode(ZIROpcode::NOP), name(name), result(""), target_label("") {}

        explicit ZIRInstructionImpl(ZIROpcode opcode, const std::string &result = "")
            : opcode(opcode), name(result), result(result), target_label("") {}

        virtual ~ZIRInstructionImpl() = default;

        // Basic accessors
        ZIROpcode getOpcode() const { return opcode; }
        const std::string &getName() const { return name; }
        void setName(const std::string &new_name) { name = new_name; }
        const std::string &getResult() const { return result; }
        void setResult(const std::string &new_result) { result = new_result; }

        // Pure virtual methods that all instructions must implement
        virtual std::string toString() const = 0;
        virtual ZIRType::Kind getResultType() const = 0;

        // Label handling
        const std::string &getTargetLabel() const { return target_label; }
        void setTargetLabel(const std::string &label) { target_label = label; }
        bool referencesLabel(const std::string &label) const
        {
            return target_label == label;
        }

        // Instruction type queries
        bool isTerminator() const
        {
            return opcode == ZIROpcode::BR ||
                   opcode == ZIROpcode::BR_COND ||
                   opcode == ZIROpcode::RET;
        }

        // Variable analysis
        std::unordered_set<std::string> getDefinedVariables() const
        {
            std::unordered_set<std::string> vars;
            if (!result.empty())
            {
                vars.insert(result);
            }
            return vars;
        }

        std::unordered_set<std::string> getUsedVariables() const
        {
            std::unordered_set<std::string> vars;
            // In a real implementation, this would analyze operands
            // For now, we'll just return an empty set
            return vars;
        }

        // Prevent copying
        ZIRInstructionImpl(const ZIRInstructionImpl &) = delete;
        ZIRInstructionImpl &operator=(const ZIRInstructionImpl &) = delete;

        // Allow moving
        ZIRInstructionImpl(ZIRInstructionImpl &&) = default;
        ZIRInstructionImpl &operator=(ZIRInstructionImpl &&) = default;

    private:
        ZIROpcode opcode;
        std::string name;   // For backward compatibility
        std::string result; // For new code
        std::string target_label;
    };

} // namespace zir

#endif // ZIR_INSTRUCTION_HPP