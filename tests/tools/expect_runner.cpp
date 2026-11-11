// The "referee" for whole-program tests (decision D49): runs a command and checks what a user
// could observe, namely its exit code, standard output and standard error.
//
//   expect_runner [options] -- <command> [arguments...]
//
// Options:
//   --exit-code <n>           expected exit code (default 0)
//   --stdout-file <path>      standard output must equal this file exactly
//   --stdout-contains <text>  standard output must contain the text (repeatable)
//   --stderr-contains <text>  standard error must contain the text (repeatable)
//   --stdin-file <path>       send this file to the command's standard input
//   --timeout <seconds>       kill the command after this long (default 60)
//   --update                  rewrite --stdout-file with the actual output instead of comparing
//                             (also enabled by the environment variable ZACK_UPDATE_GOLDEN=1)
//
// Exit codes: 0 the command behaved as expected, 1 it did not, 2 bad usage.

#include "driver/process.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr int kPass = 0;
constexpr int kFail = 1;
constexpr int kUsage = 2;

struct Options {
    int expectedExitCode = 0;
    std::optional<std::string> stdoutFile;
    std::optional<std::string> stdinFile;
    std::vector<std::string> stdoutContains;
    std::vector<std::string> stderrContains;
    double timeoutSeconds = 60;
    bool update = false;
    std::vector<std::string> command;
};

// Windows may produce CRLF line endings; expectations are stored with LF.
std::string normalizeLineEndings(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\r' && i + 1 < text.size() && text[i + 1] == '\n') {
            continue;
        }
        result.push_back(text[i]);
    }
    return result;
}

std::optional<std::string> readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return std::nullopt;
    }
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    return lines;
}

// Describes the first line where two texts differ.
std::string describeDifference(const std::string& expected, const std::string& actual) {
    std::vector<std::string> expectedLines = splitLines(expected);
    std::vector<std::string> actualLines = splitLines(actual);
    std::size_t count = std::max(expectedLines.size(), actualLines.size());
    for (std::size_t i = 0; i < count; ++i) {
        const std::string* e = i < expectedLines.size() ? &expectedLines[i] : nullptr;
        const std::string* a = i < actualLines.size() ? &actualLines[i] : nullptr;
        if (e == nullptr || a == nullptr || *e != *a) {
            std::ostringstream out;
            out << "first difference at line " << (i + 1) << ":\n"
                << "      expected: " << (e != nullptr ? "\"" + *e + "\"" : "(no such line)")
                << "\n"
                << "      actual:   " << (a != nullptr ? "\"" + *a + "\"" : "(no such line)");
            return out.str();
        }
    }
    return "the texts differ only at the end (trailing newline?)";
}

std::optional<Options> parseArguments(int argc, char** argv, std::string& error) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--") {
            for (int j = i + 1; j < argc; ++j) {
                options.command.push_back(argv[j]);
            }
            break;
        }
        auto value = [&](const char* name) -> std::optional<std::string> {
            if (i + 1 >= argc) {
                error = std::string("missing value after ") + name;
                return std::nullopt;
            }
            return std::string(argv[++i]);
        };
        if (argument == "--update") {
            options.update = true;
        } else if (argument == "--exit-code" || argument == "--timeout") {
            std::optional<std::string> text = value(argument.c_str());
            if (!text) {
                return std::nullopt;
            }
            char* end = nullptr;
            double number = std::strtod(text->c_str(), &end);
            if (end == text->c_str() || *end != '\0') {
                error = "not a number: " + *text;
                return std::nullopt;
            }
            if (argument == "--exit-code") {
                options.expectedExitCode = static_cast<int>(number);
            } else {
                options.timeoutSeconds = number;
            }
        } else if (argument == "--stdout-file" || argument == "--stdin-file" ||
                   argument == "--stdout-contains" || argument == "--stderr-contains") {
            std::optional<std::string> text = value(argument.c_str());
            if (!text) {
                return std::nullopt;
            }
            if (argument == "--stdout-file") {
                options.stdoutFile = *text;
            } else if (argument == "--stdin-file") {
                options.stdinFile = *text;
            } else if (argument == "--stdout-contains") {
                options.stdoutContains.push_back(*text);
            } else {
                options.stderrContains.push_back(*text);
            }
        } else {
            error = "unknown option " + argument;
            return std::nullopt;
        }
    }
    if (options.command.empty()) {
        error = "no command given after '--'";
        return std::nullopt;
    }
    if (const char* env = std::getenv("ZACK_UPDATE_GOLDEN"); env != nullptr && env[0] == '1') {
        options.update = true;
    }
    return options;
}

} // namespace

int main(int argc, char** argv) {
    std::string error;
    std::optional<Options> parsed = parseArguments(argc, argv, error);
    if (!parsed) {
        std::cerr << "expect_runner: " << error << "\n";
        return kUsage;
    }
    const Options& options = *parsed;

    zack::driver::CaptureOptions capture;
    capture.timeoutSeconds = options.timeoutSeconds;
    if (options.stdinFile) {
        std::optional<std::string> input = readFile(*options.stdinFile);
        if (!input) {
            std::cerr << "expect_runner: cannot read " << *options.stdinFile << "\n";
            return kUsage;
        }
        capture.standardInput = *input;
    }

    std::optional<zack::driver::ProcessResult> result =
        zack::driver::runProcessCaptured(options.command, capture);
    if (!result) {
        std::cerr << "FAIL: could not start " << options.command[0] << "\n";
        return kFail;
    }

    const std::string standardOutput = normalizeLineEndings(result->standardOutput);
    const std::string standardError = normalizeLineEndings(result->standardError);
    std::vector<std::string> failures;

    if (result->timedOut) {
        failures.push_back("timed out after " + std::to_string(options.timeoutSeconds) +
                           " seconds");
    } else if (result->exitCode != options.expectedExitCode) {
        failures.push_back("exit code: expected " + std::to_string(options.expectedExitCode) +
                           ", got " + std::to_string(result->exitCode));
    }

    if (options.stdoutFile) {
        if (options.update) {
            std::ofstream file(*options.stdoutFile, std::ios::binary);
            file << standardOutput;
            std::cout << "updated " << *options.stdoutFile << "\n";
        } else {
            std::optional<std::string> expected = readFile(*options.stdoutFile);
            if (!expected) {
                failures.push_back("cannot read expected output file " + *options.stdoutFile);
            } else if (normalizeLineEndings(*expected) != standardOutput) {
                failures.push_back(
                    "standard output differs; " +
                    describeDifference(normalizeLineEndings(*expected), standardOutput));
            }
        }
    }
    for (const std::string& needle : options.stdoutContains) {
        if (standardOutput.find(needle) == std::string::npos) {
            failures.push_back("standard output does not contain \"" + needle + "\"; it was:\n" +
                               standardOutput);
        }
    }
    for (const std::string& needle : options.stderrContains) {
        if (standardError.find(needle) == std::string::npos) {
            failures.push_back("standard error does not contain \"" + needle + "\"; it was:\n" +
                               standardError);
        }
    }

    if (failures.empty()) {
        return kPass;
    }
    std::cerr << "FAIL:";
    for (const std::string& part : options.command) {
        std::cerr << " " << part;
    }
    std::cerr << "\n";
    for (const std::string& failure : failures) {
        std::cerr << "  - " << failure << "\n";
    }
    return kFail;
}
