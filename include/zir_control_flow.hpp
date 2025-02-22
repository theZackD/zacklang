#ifndef ZIR_CONTROL_FLOW_HPP
#define ZIR_CONTROL_FLOW_HPP

#include "zir_instruction.hpp"
#include "zir_value.hpp"
#include "zir_basic_block.hpp"
#include <memory>

namespace zir
{
    // Base class for control flow instructions
    class ControlFlowInst : public ZIRInstructionImpl
    {
    public:
        explicit ControlFlowInst(const std::string &name)
            : ZIRInstructionImpl(name) {}

        // Control flow instructions don't produce values
        ZIRType::Kind getResultType() const override
        {
            return ZIRType::Kind::Void;
        }
    };

    // Conditional branch instruction (if-then-else)
    class BranchInst : public ControlFlowInst
    {
    public:
        BranchInst(std::shared_ptr<ZIRValue> condition,
                   std::shared_ptr<ZIRBasicBlockImpl> true_block,
                   std::shared_ptr<ZIRBasicBlockImpl> false_block)
            : ControlFlowInst("br"), condition(condition),
              true_block(true_block), false_block(false_block),
              true_block_handle(nullptr), false_block_handle(nullptr) {}

        std::shared_ptr<ZIRValue> getCondition() const { return condition; }
        std::shared_ptr<ZIRBasicBlockImpl> getTrueBlock() const { return true_block; }
        std::shared_ptr<ZIRBasicBlockImpl> getFalseBlock() const { return false_block; }

        void setTrueBlockHandle(void *handle) { true_block_handle = handle; }
        void setFalseBlockHandle(void *handle) { false_block_handle = handle; }
        void *getTrueBlockHandle() const { return true_block_handle; }
        void *getFalseBlockHandle() const { return false_block_handle; }

        std::string toString() const override
        {
            return "br " + condition->toString() + ", " +
                   true_block->getName() + ", " +
                   false_block->getName();
        }

    private:
        std::shared_ptr<ZIRValue> condition;
        std::shared_ptr<ZIRBasicBlockImpl> true_block;
        std::shared_ptr<ZIRBasicBlockImpl> false_block;
        void *true_block_handle;
        void *false_block_handle;
    };

    // Unconditional jump instruction
    class JumpInst : public ControlFlowInst
    {
    public:
        explicit JumpInst(std::shared_ptr<ZIRBasicBlockImpl> target)
            : ControlFlowInst("jump"), target(target), target_handle(nullptr) {}

        std::shared_ptr<ZIRBasicBlockImpl> getTarget() const { return target; }

        void setTargetHandle(void *handle) { target_handle = handle; }
        void *getTargetHandle() const { return target_handle; }

        std::string toString() const override
        {
            return "jump " + target->getName();
        }

    private:
        std::shared_ptr<ZIRBasicBlockImpl> target;
        void *target_handle;
    };

    // Return instruction (with optional value)
    class ReturnInst : public ControlFlowInst
    {
    public:
        explicit ReturnInst(std::shared_ptr<ZIRValue> value = nullptr)
            : ControlFlowInst("return"), value(value) {}

        std::shared_ptr<ZIRValue> getValue() const { return value; }

        std::string toString() const override
        {
            if (value)
            {
                return "return " + value->toString();
            }
            return "return void";
        }

    private:
        std::shared_ptr<ZIRValue> value;
    };

} // namespace zir

#endif // ZIR_CONTROL_FLOW_HPP