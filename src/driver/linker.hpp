#ifndef ZACK_DRIVER_LINKER_HPP
#define ZACK_DRIVER_LINKER_HPP

#include <optional>
#include <string>

namespace zack::driver {

// The C compiler driver used as the linker (decision D51). Taken from the ZACK_CC
// environment variable when set, otherwise "cc" on Unix-like systems and "clang" on Windows.
std::string linkerCommand();

// Links an object file with the runtime library into an executable.
// Returns an error message, or std::nullopt on success.
std::optional<std::string> linkExecutable(const std::string& objectFile,
                                          const std::string& runtimeLibrary,
                                          const std::string& outputFile);

} // namespace zack::driver

#endif // ZACK_DRIVER_LINKER_HPP
