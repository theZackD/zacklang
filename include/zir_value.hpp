#ifndef ZIR_VALUE_HPP
#define ZIR_VALUE_HPP

#include <string>
#include <memory>
#include <cstdint>
#include "zir_type.hpp"

namespace zir
{

    // Forward declarations
    class ZIRType;

    class ZIRValue
    {
    public:
        // Constructor and virtual destructor
        explicit ZIRValue(std::shared_ptr<ZIRType> type);
        virtual ~ZIRValue() = default;

        // Pure virtual methods that all values must implement
        virtual std::string toString() const = 0;
        virtual bool isConstant() const = 0;

        // Type information
        std::shared_ptr<ZIRType> getType() const { return type; }

        // Disable copy operations (values should be managed through shared_ptr)
        ZIRValue(const ZIRValue &) = delete;
        ZIRValue &operator=(const ZIRValue &) = delete;

    protected:
        std::shared_ptr<ZIRType> type;
    };

    class ZIRIntegerLiteral : public ZIRValue
    {
    public:
        ZIRIntegerLiteral(std::shared_ptr<ZIRIntegerType> type, int64_t value);

        // Implement pure virtual methods
        std::string toString() const override;
        bool isConstant() const override { return true; }

        // Value access
        int64_t getValue() const { return value; }

    private:
        int64_t value;
    };

    class ZIRFloatLiteral : public ZIRValue
    {
    public:
        ZIRFloatLiteral(std::shared_ptr<ZIRFloatType> type, double value);

        // Implement pure virtual methods
        std::string toString() const override;
        bool isConstant() const override { return true; }

        // Value access
        double getValue() const { return value; }

    private:
        double value;
    };

    class ZIRBooleanLiteral : public ZIRValue
    {
    public:
        ZIRBooleanLiteral(std::shared_ptr<ZIRBooleanType> type, bool value);

        // Implement pure virtual methods
        std::string toString() const override;
        bool isConstant() const override { return true; }

        // Value access
        bool getValue() const { return value; }

    private:
        bool value;
    };

    class ZIRStringLiteral : public ZIRValue
    {
    public:
        ZIRStringLiteral(std::shared_ptr<ZIRStringType> type, std::string value);

        // Implement pure virtual methods
        std::string toString() const override;
        bool isConstant() const override { return true; }

        // Value access
        const std::string &getValue() const { return value; }

    private:
        std::string value;
    };

} // namespace zir

#endif // ZIR_VALUE_HPP
