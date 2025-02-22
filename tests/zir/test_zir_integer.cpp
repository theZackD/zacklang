#include "../../include/zir_value.hpp"
#include "../../include/zir_type.hpp"
#include <cassert>
#include <iostream>
#include <memory>

using namespace zir;

void test_integer_type()
{
    // Test i32 type
    auto i32_type = std::make_shared<ZIRIntegerType>(ZIRIntegerType::Width::Int32);
    assert(i32_type->getKind() == ZIRType::Kind::Integer);
    assert(i32_type->getWidth() == ZIRIntegerType::Width::Int32);
    assert(i32_type->toString() == "i32");

    // Test i64 type
    auto i64_type = std::make_shared<ZIRIntegerType>(ZIRIntegerType::Width::Int64);
    assert(i64_type->getKind() == ZIRType::Kind::Integer);
    assert(i64_type->getWidth() == ZIRIntegerType::Width::Int64);
    assert(i64_type->toString() == "i64");

    std::cout << "✓ Integer type tests passed\n";
}

void test_integer_literal()
{
    // Test i32 literal
    auto i32_type = std::make_shared<ZIRIntegerType>(ZIRIntegerType::Width::Int32);
    auto i32_lit = std::make_shared<ZIRIntegerLiteral>(i32_type, 42);
    assert(i32_lit->getValue() == 42);
    assert(i32_lit->toString() == "42");
    assert(i32_lit->isConstant() == true);
    assert(std::dynamic_pointer_cast<ZIRIntegerType>(i32_lit->getType())->getWidth() == ZIRIntegerType::Width::Int32);

    // Test i64 literal
    auto i64_type = std::make_shared<ZIRIntegerType>(ZIRIntegerType::Width::Int64);
    auto i64_lit = std::make_shared<ZIRIntegerLiteral>(i64_type, 9223372036854775807LL); // Max int64
    assert(i64_lit->getValue() == 9223372036854775807LL);
    assert(i64_lit->toString() == "9223372036854775807");
    assert(i64_lit->isConstant() == true);
    assert(std::dynamic_pointer_cast<ZIRIntegerType>(i64_lit->getType())->getWidth() == ZIRIntegerType::Width::Int64);

    std::cout << "✓ Integer literal tests passed\n";
}

int main()
{
    std::cout << "Running ZIR integer tests...\n";

    test_integer_type();
    test_integer_literal();

    std::cout << "All ZIR integer tests passed!\n";
    return 0;
}