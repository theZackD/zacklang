#include "../../include/zir_value.hpp"
#include "../../include/zir_type.hpp"
#include <cassert>
#include <iostream>
#include <memory>

using namespace zir;

void test_string_type()
{
    // Test string type
    auto string_type = std::make_shared<ZIRStringType>();
    assert(string_type->getKind() == ZIRType::Kind::String);
    assert(string_type->toString() == "string");

    std::cout << "✓ String type tests passed\n";
}

void test_string_literal()
{
    auto string_type = std::make_shared<ZIRStringType>();

    // Test basic string
    auto basic_lit = std::make_shared<ZIRStringLiteral>(string_type, "Hello, World!");
    assert(basic_lit->getValue() == "Hello, World!");
    assert(basic_lit->toString() == "\"Hello, World!\"");
    assert(basic_lit->isConstant() == true);
    assert(basic_lit->getType()->getKind() == ZIRType::Kind::String);

    // Test empty string
    auto empty_lit = std::make_shared<ZIRStringLiteral>(string_type, "");
    assert(empty_lit->getValue() == "");
    assert(empty_lit->toString() == "\"\"");

    // Test string with escape characters
    auto escaped_lit = std::make_shared<ZIRStringLiteral>(string_type, "Line1\nLine2\t\"quoted\"\\backslash");
    assert(escaped_lit->toString() == "\"Line1\\nLine2\\t\\\"quoted\\\"\\\\backslash\"");

    std::cout << "✓ String literal tests passed\n";
}

int main()
{
    std::cout << "Running ZIR string tests...\n";

    test_string_type();
    test_string_literal();

    std::cout << "All ZIR string tests passed!\n";
    return 0;
}