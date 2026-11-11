#include "driver/linker.hpp"

#include "driver/process.hpp"

#include <cstdlib>

namespace zack::driver {

std::string linkerCommand() {
    if (const char* fromEnvironment = std::getenv("ZACK_CC");
        fromEnvironment != nullptr && fromEnvironment[0] != '\0') {
        return fromEnvironment;
    }
#if defined(_WIN32)
    return "clang";
#else
    return "cc";
#endif
}

std::optional<std::string> linkExecutable(const std::string& objectFile,
                                          const std::string& runtimeLibrary,
                                          const std::string& outputFile) {
    const std::string linker = linkerCommand();
    std::optional<int> exitCode =
        runProcess({linker, objectFile, runtimeLibrary, "-o", outputFile});
    if (!exitCode.has_value()) {
        return "could not start the linker '" + linker +
               "'; set ZACK_CC to a C compiler driver such as clang";
    }
    if (*exitCode != 0) {
        return "the linker '" + linker + "' failed with exit code " + std::to_string(*exitCode);
    }
    return std::nullopt;
}

} // namespace zack::driver
