#ifndef ZIR_ARITHMETIC_HPP
#define ZIR_ARITHMETIC_HPP

#include "zir_instruction.hpp"
#include "zir_value.hpp"
#include <memory>
#include <unordered_set>

namespace zir
{

    class BinaryArithmeticInst : public ZIRInstructionImpl
    {
    public:
        BinaryArithmeticInst(ZIROpcode opcode,
                             const std::string &name,
                             std::shared_ptr<ZIRValue> left,
                             std::shared_ptr<ZIRValue> right)
            : ZIRInstructionImpl(opcode, name), left(left), right(right) {}

        std::shared_ptr<ZIRValue> getLeft() const { return left; }
        std::shared_ptr<ZIRValue> getRight() const { return right; }

        // Default string representation for binary operations
        std::string toString() const override
        {
            return left->toString() + " " + getName() + " " + right->toString();
        }

        // Default to using left operand's type
        ZIRType::Kind getResultType() const override
        {
            return left->getType()->getKind();
        }

        // Override getUsedVariables to return the names of operands
        std::unordered_set<std::string> getUsedVariables() const override
        {
            std::unordered_set<std::string> vars;
            // Add both operands' names if they are ZIRInstructionImpl results
            if (auto leftInst = std::dynamic_pointer_cast<ZIRInstructionImpl>(left))
            {
                if (!leftInst->getResult().empty())
                {
                    vars.insert(leftInst->getResult());
                }
            }
            else if (left)
            {
                // If not an instruction, use the toString representation
                vars.insert(left->toString());
            }

            if (auto rightInst = std::dynamic_pointer_cast<ZIRInstructionImpl>(right))
            {
                if (!rightInst->getResult().empty())
                {
                    vars.insert(rightInst->getResult());
                }
            }
            else if (right)
            {
                // If not an instruction, use the toString representation
                vars.insert(right->toString());
            }

            return vars;
        }

    protected:
        std::shared_ptr<ZIRValue> left;
        std::shared_ptr<ZIRValue> right;
    };

    class AddInst : public BinaryArithmeticInst
    {
    public:
        AddInst(std::shared_ptr<ZIRValue> left, std::shared_ptr<ZIRValue> right)
            : BinaryArithmeticInst(ZIROpcode::ADD, "add", left, right) {}
    };

    class SubInst : public BinaryArithmeticInst
    {
    public:
        SubInst(std::shared_ptr<ZIRValue> left, std::shared_ptr<ZIRValue> right)
            : BinaryArithmeticInst(ZIROpcode::SUB, "sub", left, right) {}
    };

    class MulInst : public BinaryArithmeticInst
    {
    public:
        MulInst(std::shared_ptr<ZIRValue> left, std::shared_ptr<ZIRValue> right)
            : BinaryArithmeticInst(ZIROpcode::MUL, "mul", left, right) {}
    };

    class DivInst : public BinaryArithmeticInst
    {
    public:
        DivInst(std::shared_ptr<ZIRValue> left, std::shared_ptr<ZIRValue> right)
            : BinaryArithmeticInst(ZIROpcode::DIV, "div", left, right) {}
    };

    class ModInst : public BinaryArithmeticInst
    {
    public:
        ModInst(std::shared_ptr<ZIRValue> left, std::shared_ptr<ZIRValue> right)
            : BinaryArithmeticInst(ZIROpcode::DIV, "mod", left, right) {}
    };

    class PowInst : public BinaryArithmeticInst
    {
    public:
        PowInst(std::shared_ptr<ZIRValue> left, std::shared_ptr<ZIRValue> right)
            : BinaryArithmeticInst(ZIROpcode::DIV, "pow", left, right) {}
    };

} // namespace zir

#endif // ZIR_ARITHMETIC_HPP