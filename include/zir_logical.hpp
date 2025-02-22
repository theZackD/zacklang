#ifndef ZIR_LOGICAL_HPP
#define ZIR_LOGICAL_HPP

#include "zir_instruction.hpp"
#include "zir_value.hpp"
#include <memory>

namespace zir
{

    // Base class for binary logical operations (AND, OR)
    class BinaryLogicalInst : public ZIRInstructionImpl
    {
    public:
        BinaryLogicalInst(const std::string &name,
                          std::shared_ptr<ZIRValue> left,
                          std::shared_ptr<ZIRValue> right)
            : ZIRInstructionImpl(name), left(left), right(right) {}

        std::shared_ptr<ZIRValue> getLeft() const { return left; }
        std::shared_ptr<ZIRValue> getRight() const { return right; }

        // Logical operations always return boolean
        ZIRType::Kind getResultType() const override
        {
            return ZIRType::Kind::Boolean;
        }

        // Default string representation for logical operations
        std::string toString() const override
        {
            return left->toString() + " " + getName() + " " + right->toString();
        }

    protected:
        std::shared_ptr<ZIRValue> left;
        std::shared_ptr<ZIRValue> right;
    };

    // Base class for unary logical operations (NOT)
    class UnaryLogicalInst : public ZIRInstructionImpl
    {
    public:
        UnaryLogicalInst(const std::string &name,
                         std::shared_ptr<ZIRValue> operand)
            : ZIRInstructionImpl(name), operand(operand) {}

        std::shared_ptr<ZIRValue> getOperand() const { return operand; }

        // Logical operations always return boolean
        ZIRType::Kind getResultType() const override
        {
            return ZIRType::Kind::Boolean;
        }

        // Default string representation for unary operations
        std::string toString() const override
        {
            return getName() + " " + operand->toString();
        }

    protected:
        std::shared_ptr<ZIRValue> operand;
    };

    // Concrete logical instruction classes
    class AndInst : public BinaryLogicalInst
    {
    public:
        AndInst(std::shared_ptr<ZIRValue> left, std::shared_ptr<ZIRValue> right)
            : BinaryLogicalInst("and", left, right) {}
    };

    class OrInst : public BinaryLogicalInst
    {
    public:
        OrInst(std::shared_ptr<ZIRValue> left, std::shared_ptr<ZIRValue> right)
            : BinaryLogicalInst("or", left, right) {}
    };

    class NotInst : public UnaryLogicalInst
    {
    public:
        NotInst(std::shared_ptr<ZIRValue> operand)
            : UnaryLogicalInst("not", operand) {}
    };

} // namespace zir

#endif // ZIR_LOGICAL_HPP