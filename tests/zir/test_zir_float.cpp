#include "../../include/zir_value.hpp"
#include "../../include/zir_type.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <cmath>

using namespace zir;

void test_float_type()
{
    // Test f32 type
    auto f32_type = std::make_shared<ZIRFloatType>(ZIRFloatType::Width::Float32);
    assert(f32_type->getKind() == ZIRType::Kind::Float);
    assert(f32_type->getWidth() == ZIRFloatType::Width::Float32);
    assert(f32_type->toString() == "f32");

    // Test f64 type
    auto f64_type = std::make_shared<ZIRFloatType>(ZIRFloatType::Width::Float64);
    assert(f64_type->getKind() == ZIRType::Kind::Float);
    assert(f64_type->getWidth() == ZIRFloatType::Width::Float64);
    assert(f64_type->toString() == "f64");

    std::cout << "✓ Float type tests passed\n";
}

void test_float_literal()
{
    // Test f32 literal
    auto f32_type = std::make_shared<ZIRFloatType>(ZIRFloatType::Width::Float32);
    auto f32_lit = std::make_shared<ZIRFloatLiteral>(f32_type, 3.14159f);
    assert(std::abs(f32_lit->getValue() - 3.14159) < 0.000001);
    assert(f32_lit->toString() == "3.14159");
    assert(f32_lit->isConstant() == true);
    assert(std::dynamic_pointer_cast<ZIRFloatType>(f32_lit->getType())->getWidth() == ZIRFloatType::Width::Float32);

    // Test f64 literal
    auto f64_type = std::make_shared<ZIRFloatType>(ZIRFloatType::Width::Float64);
    auto f64_lit = std::make_shared<ZIRFloatLiteral>(f64_type, 3.14159265359);
    assert(std::abs(f64_lit->getValue() - 3.14159265359) < 0.000000000001);
    assert(f64_lit->toString() == "3.141593"); // Default precision is 6 decimal places
    assert(f64_lit->isConstant() == true);
    assert(std::dynamic_pointer_cast<ZIRFloatType>(f64_lit->getType())->getWidth() == ZIRFloatType::Width::Float64);

    std::cout << "✓ Float literal tests passed\n";
}

void test_float_formatting()
{
    auto f64_type = std::make_shared<ZIRFloatType>(ZIRFloatType::Width::Float64);

    // Test whole numbers
    auto whole = std::make_shared<ZIRFloatLiteral>(f64_type, 42.0);
    assert(whole->toString() == "42.0");

    // Test small decimals
    auto small = std::make_shared<ZIRFloatLiteral>(f64_type, 0.000123);
    assert(small->toString() == "0.000123");

    // Test trailing zeros
    auto trailing = std::make_shared<ZIRFloatLiteral>(f64_type, 3.1400);
    assert(trailing->toString() == "3.14");

    std::cout << "✓ Float formatting tests passed\n";
}

int main()
{
    std::cout << "Running ZIR float tests...\n";

    test_float_type();
    test_float_literal();
    test_float_formatting();

    std::cout << "All ZIR float tests passed!\n";
    return 0;
}