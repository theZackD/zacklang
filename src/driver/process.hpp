#ifndef ZACK_DRIVER_PROCESS_HPP
#define ZACK_DRIVER_PROCESS_HPP

#include <optional>
#include <string>
#include <vector>

namespace zack::driver {

// Runs `arguments[0]` (looked up on PATH) with the remaining arguments and waits for it. The
// child shares this process's standard input, output and error. Arguments are passed as an
// array, never through a shell, so paths with spaces are safe.
// Returns the exit code, or std::nullopt when the program could not be started.
std::optional<int> runProcess(const std::vector<std::string>& arguments);

struct CaptureOptions {
    // Sent to the child's standard input, which is then closed. An empty string simply
    // gives the child an empty input (it sees end-of-file at once).
    std::string standardInput;
    // Kills the child after this many seconds. Zero or negative means no limit.
    double timeoutSeconds = 0;
};

struct ProcessResult {
    int exitCode = 0; // a child killed by a signal reports 128 + the signal number
    std::string standardOutput;
    std::string standardError;
    bool timedOut = false;
};

// Like runProcess, but collects the child's standard output and standard error.
// Returns std::nullopt when the program could not be started.
std::optional<ProcessResult> runProcessCaptured(const std::vector<std::string>& arguments,
                                                const CaptureOptions& options = {});

} // namespace zack::driver

#endif // ZACK_DRIVER_PROCESS_HPP
