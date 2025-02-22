#include "../../include/zir_value.hpp"
#include "../../include/zir_type.hpp"
#include <cassert>
#include <iostream>
#include <memory>

using namespace zir;

void test_boolean_type()
{
    // Test boolean type
    auto bool_type = std::make_shared<ZIRBooleanType>();
    assert(bool_type->getKind() == ZIRType::Kind::Boolean);
    assert(bool_type->toString() == "bool");

    std::cout << "✓ Boolean type tests passed\n";
}

void test_boolean_literal()
{
    auto bool_type = std::make_shared<ZIRBooleanType>();

    // Test true literal
    auto true_lit = std::make_shared<ZIRBooleanLiteral>(bool_type, true);
    assert(true_lit->getValue() == true);
    assert(true_lit->toString() == "true");
    assert(true_lit->isConstant() == true);
    assert(true_lit->getType()->getKind() == ZIRType::Kind::Boolean);

    // Test false literal
    auto false_lit = std::make_shared<ZIRBooleanLiteral>(bool_type, false);
    assert(false_lit->getValue() == false);
    assert(false_lit->toString() == "false");
    assert(false_lit->isConstant() == true);
    assert(false_lit->getType()->getKind() == ZIRType::Kind::Boolean);

    std::cout << "✓ Boolean literal tests passed\n";
}

int main()
{
    std::cout << "Running ZIR boolean tests...\n";

    test_boolean_type();
    test_boolean_literal();

    std::cout << "All ZIR boolean tests passed!\n";
    return 0;
}