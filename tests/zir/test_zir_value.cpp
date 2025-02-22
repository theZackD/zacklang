#include "../../include/zir_value.hpp"
#include "../../include/zir_type.hpp"
#include <cassert>
#include <iostream>

// Test concrete implementation of ZIRValue for testing
class TestValue : public zir::ZIRValue
{
public:
    explicit TestValue(std::shared_ptr<zir::ZIRType> type)
        : ZIRValue(std::move(type)) {}

    std::string toString() const override { return "test"; }
    bool isConstant() const override { return true; }
};

// Test concrete implementation of ZIRType for testing
class TestType : public zir::ZIRType
{
public:
    TestType() : ZIRType(Kind::Integer) {}
    std::string toString() const override { return "test_type"; }
};

void test_value_basics()
{
    // Create a type
    auto type = std::make_shared<TestType>();

    // Create a value
    auto value = std::make_shared<TestValue>(type);

    // Test type access
    assert(value->getType() == type);
    assert(value->getType()->getKind() == zir::ZIRType::Kind::Integer);

    // Test virtual methods
    assert(value->toString() == "test");
    assert(value->isConstant() == true);

    std::cout << "✓ Basic value tests passed\n";
}

int main()
{
    std::cout << "Running ZIR value tests...\n";

    test_value_basics();

    std::cout << "All ZIR value tests passed!\n";
    return 0;
}
