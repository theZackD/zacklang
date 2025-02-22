#include "../include/zir_value.hpp"
#include "../include/zir_type.hpp"
#include <iomanip>
#include <sstream>

namespace zir
{

    ZIRValue::ZIRValue(std::shared_ptr<ZIRType> type)
        : type(std::move(type))
    {
    }

    std::string ZIRIntegerType::toString() const
    {
        switch (getWidth())
        {
        case Width::Int32:
            return "i32";
        case Width::Int64:
            return "i64";
        default:
            return "unknown";
        }
    }

    std::string ZIRFloatType::toString() const
    {
        switch (getWidth())
        {
        case Width::Float32:
            return "f32";
        case Width::Float64:
            return "f64";
        default:
            return "unknown";
        }
    }

    ZIRIntegerLiteral::ZIRIntegerLiteral(std::shared_ptr<ZIRIntegerType> type, int64_t value)
        : ZIRValue(type), value(value)
    {
    }

    std::string ZIRIntegerLiteral::toString() const
    {
        return std::to_string(value);
    }

    ZIRFloatLiteral::ZIRFloatLiteral(std::shared_ptr<ZIRFloatType> type, double value)
        : ZIRValue(type), value(value)
    {
    }

    std::string ZIRFloatLiteral::toString() const
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6) << value;
        std::string str = oss.str();

        // Remove trailing zeros after decimal point
        size_t decimal_pos = str.find('.');
        if (decimal_pos != std::string::npos)
        {
            str.erase(str.find_last_not_of('0') + 1);
            if (str.back() == '.')
            {
                str.push_back('0');
            }
        }

        return str;
    }

    std::string ZIRBooleanType::toString() const
    {
        return "bool";
    }

    ZIRBooleanLiteral::ZIRBooleanLiteral(std::shared_ptr<ZIRBooleanType> type, bool value)
        : ZIRValue(type), value(value)
    {
    }

    std::string ZIRBooleanLiteral::toString() const
    {
        return value ? "true" : "false";
    }

    std::string ZIRStringType::toString() const
    {
        return "string";
    }

    ZIRStringLiteral::ZIRStringLiteral(std::shared_ptr<ZIRStringType> type, std::string value)
        : ZIRValue(type), value(std::move(value))
    {
    }

    std::string ZIRStringLiteral::toString() const
    {
        // Add quotes and escape special characters
        std::string result = "\"";
        for (char c : value)
        {
            switch (c)
            {
            case '\"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                result += c;
            }
        }
        result += "\"";
        return result;
    }

} // namespace zir
