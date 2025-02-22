#ifndef ZIR_BUILDER_HPP
#define ZIR_BUILDER_HPP

#include <string>
#include <memory>
#include "zir_type.hpp"
#include "zir_value.hpp"

namespace zir
{

    class ZIRBuilderImpl
    {
    public:
        ZIRBuilderImpl() = default;
        ~ZIRBuilderImpl() = default;

        // Prevent copying
        ZIRBuilderImpl(const ZIRBuilderImpl &) = delete;
        ZIRBuilderImpl &operator=(const ZIRBuilderImpl &) = delete;

        // Allow moving
        ZIRBuilderImpl(ZIRBuilderImpl &&) = default;
        ZIRBuilderImpl &operator=(ZIRBuilderImpl &&) = default;

        // Version information
        std::string getVersion() const { return "0.1.0"; }

        // Type creation methods
        std::shared_ptr<ZIRIntegerType> createI32Type()
        {
            return std::make_shared<ZIRIntegerType>(ZIRIntegerType::Width::Int32);
        }

        std::shared_ptr<ZIRIntegerType> createI64Type()
        {
            return std::make_shared<ZIRIntegerType>(ZIRIntegerType::Width::Int64);
        }

        std::shared_ptr<ZIRFloatType> createF32Type()
        {
            return std::make_shared<ZIRFloatType>(ZIRFloatType::Width::Float32);
        }

        std::shared_ptr<ZIRFloatType> createF64Type()
        {
            return std::make_shared<ZIRFloatType>(ZIRFloatType::Width::Float64);
        }

        std::shared_ptr<ZIRBooleanType> createBoolType()
        {
            return std::make_shared<ZIRBooleanType>();
        }

        std::shared_ptr<ZIRStringType> createStringType()
        {
            return std::make_shared<ZIRStringType>();
        }

        // Literal creation methods
        std::shared_ptr<ZIRIntegerLiteral> createIntegerLiteral(std::shared_ptr<ZIRIntegerType> type, int64_t value)
        {
            return std::make_shared<ZIRIntegerLiteral>(type, value);
        }

        std::shared_ptr<ZIRFloatLiteral> createFloatLiteral(std::shared_ptr<ZIRFloatType> type, double value)
        {
            return std::make_shared<ZIRFloatLiteral>(type, value);
        }

        std::shared_ptr<ZIRBooleanLiteral> createBoolLiteral(std::shared_ptr<ZIRBooleanType> type, bool value)
        {
            return std::make_shared<ZIRBooleanLiteral>(type, value);
        }

        std::shared_ptr<ZIRStringLiteral> createStringLiteral(std::shared_ptr<ZIRStringType> type, const std::string &value)
        {
            return std::make_shared<ZIRStringLiteral>(type, value);
        }
    };

} // namespace zir

#endif // ZIR_BUILDER_HPP