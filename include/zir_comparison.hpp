#ifndef ZIR_COMPARISON_HPP
#define ZIR_COMPARISON_HPP

#include "zir_instruction.hpp"
#include "zir_value.hpp"
#include <memory>

namespace zir
{

    class BinaryComparisonInst : public ZIRInstructionImpl
    {
    public:
        BinaryComparisonInst(const std::string &name,
                             std::shared_ptr<ZIRValue> left,
                             std::shared_ptr<ZIRValue> right)
            : ZIRInstructionImpl(name), left(left), right(right) {}

        std::shared_ptr<ZIRValue> getLeft() const { return left; }
        std::shared_ptr<ZIRValue> getRight() const { return right; }

        // Comparison operations always return boolean
        ZIRType::Kind getResultType() const override
        {
            return ZIRType::Kind::Boolean;
        }

        // Default string representation for comparison operations
        std::string toString() const override
        {
            return left->toString() + " " + getName() + " " + right->toString();
        }

    protected:
        std::shared_ptr<ZIRValue> left;
        std::shared_ptr<ZIRValue> right;
    };

    class EqInst : public BinaryComparisonInst
    {
    public:
        EqInst(std::shared_ptr<ZIRValue> left, std::shared_ptr<ZIRValue> right)
            : BinaryComparisonInst("eq", left, right) {}
    };

    class NeInst : public BinaryComparisonInst
    {
    public:
        NeInst(std::shared_ptr<ZIRValue> left, std::shared_ptr<ZIRValue> right)
            : BinaryComparisonInst("ne", left, right) {}
    };

    class LtInst : public BinaryComparisonInst
    {
    public:
        LtInst(std::shared_ptr<ZIRValue> left, std::shared_ptr<ZIRValue> right)
            : BinaryComparisonInst("lt", left, right) {}
    };

    class LeInst : public BinaryComparisonInst
    {
    public:
        LeInst(std::shared_ptr<ZIRValue> left, std::shared_ptr<ZIRValue> right)
            : BinaryComparisonInst("le", left, right) {}
    };

    class GtInst : public BinaryComparisonInst
    {
    public:
        GtInst(std::shared_ptr<ZIRValue> left, std::shared_ptr<ZIRValue> right)
            : BinaryComparisonInst("gt", left, right) {}
    };

    class GeInst : public BinaryComparisonInst
    {
    public:
        GeInst(std::shared_ptr<ZIRValue> left, std::shared_ptr<ZIRValue> right)
            : BinaryComparisonInst("ge", left, right) {}
    };

} // namespace zir

#endif // ZIR_COMPARISON_HPP