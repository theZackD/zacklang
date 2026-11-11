#include "codegen/backend.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

using zack::codegen::Backend;
using zack::codegen::Function;
using zack::codegen::Type;

TEST_CASE("the linked LLVM is the pinned major version", "[backend]") {
    REQUIRE(zack::codegen::llvmMajorVersion() == 20u);
    REQUIRE(zack::codegen::llvmVersionString().rfind("LLVM 20.", 0) == 0);
}

TEST_CASE("a built module verifies and contains the function", "[backend]") {
    Backend backend("unit");
    Function main = backend.declareFunction("main", Type::I32, {});
    backend.beginBody(main);
    backend.ret(backend.add(backend.constI32(40), backend.constI32(2)));
    REQUIRE_FALSE(backend.verify().has_value());
    REQUIRE(backend.irText().find("@main") != std::string::npos);
}

TEST_CASE("an invalid module is reported, not a crash", "[backend]") {
    Backend backend("unit");
    Function main = backend.declareFunction("main", Type::I32, {});
    backend.beginBody(main); // the entry block has no terminator, so it is invalid
    REQUIRE(backend.verify().has_value());
}

TEST_CASE("pass pipelines: a valid one runs, an invalid one is reported", "[backend]") {
    Backend backend("unit");
    Function main = backend.declareFunction("main", Type::I32, {});
    backend.beginBody(main);
    backend.ret(backend.constI32(0));
    REQUIRE_FALSE(backend.runPasses("mem2reg").has_value());
    REQUIRE(backend.runPasses("this-is-not-a-pass").has_value());
}

TEST_CASE("a void call returns an invalid value and the module still verifies", "[backend]") {
    Backend backend("unit");
    Function print = backend.declareFunction("zack_print_i32", Type::Void, {Type::I32});
    Function main = backend.declareFunction("main", Type::I32, {});
    backend.beginBody(main);
    auto result = backend.call(print, {backend.constI32(1)});
    REQUIRE(result.id == UINT32_MAX);
    backend.ret(backend.constI32(0));
    REQUIRE_FALSE(backend.verify().has_value());
}
