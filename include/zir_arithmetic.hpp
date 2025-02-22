#ifndef ZIR_ARITHMETIC_HPP
#define ZIR_ARITHMETIC_HPP

#include "zir_instruction.hpp"
#include "zir_value.hpp"
#include <memory>

namespace zir
{

    class BinaryArithmeticInst : public ZIRInstructionImpl
    {
    public:
        BinaryArithmeticInst(const std::string &name,
                             std::shared_ptr<ZIRValue> left,
                             std::shared_ptr<ZIRValue> right)
            : ZIRInstructionImpl(name), left(left), right(right) {}

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

    protected:
        std::shared_ptr<ZIRValue> left;
        std::shared_ptr<ZIRValue> right;
    };

    class AddInst : public BinaryArithmeticInst
    {
    public:
        AddInst(std::shared_ptr<ZIRValue> left, std::shared_ptr<ZIRValue> right)
            : BinaryArithmeticInst("add", left, right) {}
    };

    class SubInst : public BinaryArithmeticInst
    {
    public:
        SubInst(std::shared_ptr<ZIRValue> left, std::shared_ptr<ZIRValue> right)
            : BinaryArithmeticInst("sub", left, right) {}
    };

    class MulInst : public BinaryArithmeticInst
    {
    public:
        MulInst(std::shared_ptr<ZIRValue> left, std::shared_ptr<ZIRValue> right)
            : BinaryArithmeticInst("mul", left, right) {}
    };

    class DivInst : public BinaryArithmeticInst
    {
    public:
        DivInst(std::shared_ptr<ZIRValue> left, std::shared_ptr<ZIRValue> right)
            : BinaryArithmeticInst("div", left, right) {}
    };

    class ModInst : public BinaryArithmeticInst
    {
    public:
        ModInst(std::shared_ptr<ZIRValue> left, std::shared_ptr<ZIRValue> right)
            : BinaryArithmeticInst("mod", left, right) {}
    };

    class PowInst : public BinaryArithmeticInst
    {
    public:
        PowInst(std::shared_ptr<ZIRValue> left, std::shared_ptr<ZIRValue> right)
            : BinaryArithmeticInst("pow", left, right) {}
    };

} // namespace zir

#endif // ZIR_ARITHMETIC_HPP