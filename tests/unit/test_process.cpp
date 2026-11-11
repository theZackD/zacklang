#include "driver/linker.hpp"
#include "driver/process.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <string>

using zack::driver::CaptureOptions;
using zack::driver::runProcess;
using zack::driver::runProcessCaptured;

namespace {

// Paths of programs built by this project are passed in through the environment by CTest,
// so no path is ever written into a C++ string literal (Windows backslashes would break it).
std::string requiredEnvironment(const char* name) {
    const char* value = std::getenv(name);
    INFO("the environment variable " << name << " must be set (run the tests through ctest)");
    REQUIRE(value != nullptr);
    return value;
}

} // namespace

TEST_CASE("runProcess reports the exit code of the child", "[process]") {
    const std::string helper = requiredEnvironment("ZACK_PROCESS_HELPER");
    REQUIRE(runProcess({helper, "exit", "0"}) == 0);
    REQUIRE(runProcess({helper, "exit", "7"}) == 7);
}

TEST_CASE("runProcess reports a program that cannot be started", "[process]") {
    REQUIRE_FALSE(runProcess({"zack-no-such-program-exists"}).has_value());
    REQUIRE_FALSE(runProcess({}).has_value());
}

TEST_CASE("runProcessCaptured collects output, error and exit code", "[process]") {
    const std::string helper = requiredEnvironment("ZACK_PROCESS_HELPER");
    auto result = runProcessCaptured({helper, "out-err"});
    REQUIRE(result.has_value());
    REQUIRE(result->exitCode == 0);
    REQUIRE(result->standardOutput == "out");
    REQUIRE(result->standardError == "err");
    REQUIRE_FALSE(result->timedOut);
}

TEST_CASE("runProcessCaptured feeds standard input", "[process]") {
    const std::string helper = requiredEnvironment("ZACK_PROCESS_HELPER");
    CaptureOptions options;
    options.standardInput = "héllo\nsecond line\n";
    auto result = runProcessCaptured({helper, "echo-stdin"}, options);
    REQUIRE(result.has_value());
    REQUIRE(result->standardOutput == options.standardInput);
}

TEST_CASE("runProcessCaptured gives a child with no input an immediate end of file", "[process]") {
    const std::string helper = requiredEnvironment("ZACK_PROCESS_HELPER");
    auto result = runProcessCaptured({helper, "echo-stdin"});
    REQUIRE(result.has_value());
    REQUIRE(result->standardOutput.empty());
}

TEST_CASE("runProcessCaptured does not deadlock on large output on both streams", "[process]") {
    const std::string helper = requiredEnvironment("ZACK_PROCESS_HELPER");
    constexpr std::size_t kSize = 2'000'000; // far more than a pipe buffer holds
    auto result = runProcessCaptured({helper, "flood", std::to_string(kSize)});
    REQUIRE(result.has_value());
    REQUIRE(result->standardOutput.size() == kSize);
    REQUIRE(result->standardError.size() == kSize);
}

TEST_CASE("runProcessCaptured survives a large input the child never reads", "[process]") {
    const std::string helper = requiredEnvironment("ZACK_PROCESS_HELPER");
    CaptureOptions options;
    options.standardInput.assign(3'000'000, 'x');
    auto result = runProcessCaptured({helper, "exit", "3"}, options);
    REQUIRE(result.has_value());
    REQUIRE(result->exitCode == 3);
}

TEST_CASE("runProcessCaptured kills a child that runs too long", "[process]") {
    const std::string helper = requiredEnvironment("ZACK_PROCESS_HELPER");
    CaptureOptions options;
    options.timeoutSeconds = 0.3;
    auto result = runProcessCaptured({helper, "sleep", "30"}, options);
    REQUIRE(result.has_value());
    REQUIRE(result->timedOut);
}

TEST_CASE("runProcessCaptured reports a program that cannot be started", "[process]") {
    REQUIRE_FALSE(runProcessCaptured({"zack-no-such-program-exists"}).has_value());
}

TEST_CASE("the linker command can be overridden and has a sensible default", "[linker]") {
    REQUIRE_FALSE(zack::driver::linkerCommand().empty());
}

TEST_CASE("linking reports a missing linker as an error message", "[linker]") {
    // The environment override is read at call time, so point it at a program that is not there.
#if defined(_WIN32)
    _putenv_s("ZACK_CC", "zack-no-such-linker");
#else
    setenv("ZACK_CC", "zack-no-such-linker", 1);
#endif
    auto error = zack::driver::linkExecutable("a.o", "libzackrt.a", "out");
#if defined(_WIN32)
    _putenv_s("ZACK_CC", "");
#else
    unsetenv("ZACK_CC");
#endif
    REQUIRE(error.has_value());
    REQUIRE(error->find("zack-no-such-linker") != std::string::npos);
}
