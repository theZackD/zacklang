#ifndef ZIR_VALUE_IMPL_HPP
#define ZIR_VALUE_IMPL_HPP

#include "zir_value.hpp"
#include "zir_type.hpp"
#include <memory>
#include <string>
#include <cstdint>

namespace zir
{
    class ZIRValueImpl : public ZIRValue
    {
    public:
        explicit ZIRValueImpl(std::shared_ptr<ZIRType> type) : ZIRValue(type) {}
        virtual ~ZIRValueImpl() = default;
        virtual std::string toString() const override = 0;
        bool isConstant() const override { return true; }
    };

    class ZIRInt32Value : public ZIRValueImpl
    {
    public:
        explicit ZIRInt32Value(int32_t value)
            : ZIRValueImpl(std::make_shared<ZIRIntegerType>(ZIRIntegerType::Width::Int32)), value_(value) {}
        int32_t getValue() const { return value_; }
        std::string toString() const override
        {
            return std::to_string(value_);
        }

    private:
        int32_t value_;
    };

    class ZIRInt64Value : public ZIRValueImpl
    {
    public:
        explicit ZIRInt64Value(int64_t value)
            : ZIRValueImpl(std::make_shared<ZIRIntegerType>(ZIRIntegerType::Width::Int64)), value_(value) {}
        int64_t getValue() const { return value_; }
        std::string toString() const override
        {
            return std::to_string(value_);
        }

    private:
        int64_t value_;
    };

    class ZIRFloatValue : public ZIRValueImpl
    {
    public:
        explicit ZIRFloatValue(float value)
            : ZIRValueImpl(std::make_shared<ZIRFloatType>(ZIRFloatType::Width::Float32)), value_(value) {}
        float getValue() const { return value_; }
        std::string toString() const override
        {
            return std::to_string(value_);
        }

    private:
        float value_;
    };

    class ZIRDoubleValue : public ZIRValueImpl
    {
    public:
        explicit ZIRDoubleValue(double value)
            : ZIRValueImpl(std::make_shared<ZIRFloatType>(ZIRFloatType::Width::Float64)), value_(value) {}
        double getValue() const { return value_; }
        std::string toString() const override
        {
            return std::to_string(value_);
        }

    private:
        double value_;
    };

    class ZIRBoolValue : public ZIRValueImpl
    {
    public:
        explicit ZIRBoolValue(bool value)
            : ZIRValueImpl(std::make_shared<ZIRBooleanType>()), value_(value) {}
        bool getValue() const { return value_; }
        std::string toString() const override
        {
            return value_ ? "true" : "false";
        }

    private:
        bool value_;
    };

} // namespace zir

#endif // ZIR_VALUE_IMPL_HPP