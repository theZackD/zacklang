#ifndef ZIR_TYPE_HPP
#define ZIR_TYPE_HPP

#include <string>

namespace zir
{

    class ZIRType
    {
    public:
        enum class Kind
        {
            Void,
            Integer,
            Float,
            Boolean,
            String
        };

        explicit ZIRType(Kind kind) : kind(kind) {}
        virtual ~ZIRType() = default;

        Kind getKind() const { return kind; }
        virtual std::string toString() const = 0;

    private:
        Kind kind;
    };

    class ZIRIntegerType : public ZIRType
    {
    public:
        enum class Width
        {
            Int32,
            Int64
        };

        explicit ZIRIntegerType(Width width)
            : ZIRType(Kind::Integer), width(width) {}

        Width getWidth() const { return width; }
        std::string toString() const override;

    private:
        Width width;
    };

    class ZIRFloatType : public ZIRType
    {
    public:
        enum class Width
        {
            Float32,
            Float64
        };

        explicit ZIRFloatType(Width width)
            : ZIRType(Kind::Float), width(width) {}

        Width getWidth() const { return width; }
        std::string toString() const override;

    private:
        Width width;
    };

    class ZIRBooleanType : public ZIRType
    {
    public:
        ZIRBooleanType()
            : ZIRType(Kind::Boolean) {}

        std::string toString() const override;
    };

    class ZIRStringType : public ZIRType
    {
    public:
        ZIRStringType()
            : ZIRType(Kind::String) {}

        std::string toString() const override;
    };

} // namespace zir

#endif // ZIR_TYPE_HPP
