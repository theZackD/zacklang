#include "driver/process.hpp"

#include <algorithm>
#include <chrono>
#include <thread>

#if defined(_WIN32)
#include <windows.h>
#else
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>

extern char** environ;
#endif

namespace zack::driver {

#if defined(_WIN32)

namespace {

// Quotes one argument following the rules used by CommandLineToArgvW and the Microsoft C
// runtime: backslashes are literal unless they precede a double quote.
std::string quoteWindowsArgument(const std::string& argument) {
    if (!argument.empty() && argument.find_first_of(" \t\n\v\"") == std::string::npos) {
        return argument;
    }
    std::string quoted = "\"";
    std::size_t backslashes = 0;
    for (char c : argument) {
        if (c == '\\') {
            ++backslashes;
            continue;
        }
        if (c == '"') {
            quoted.append(backslashes * 2 + 1, '\\');
            quoted.push_back('"');
        } else {
            quoted.append(backslashes, '\\');
            quoted.push_back(c);
        }
        backslashes = 0;
    }
    quoted.append(backslashes * 2, '\\');
    quoted.push_back('"');
    return quoted;
}

std::string windowsCommandLine(const std::vector<std::string>& arguments) {
    std::string commandLine;
    for (const std::string& argument : arguments) {
        if (!commandLine.empty()) {
            commandLine.push_back(' ');
        }
        commandLine += quoteWindowsArgument(argument);
    }
    return commandLine;
}

// Owns a Windows HANDLE.
class Handle {
public:
    Handle() = default;
    explicit Handle(HANDLE handle) : handle_(handle) {}
    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;
    Handle(Handle&& other) noexcept : handle_(other.handle_) { other.handle_ = nullptr; }
    Handle& operator=(Handle&& other) noexcept {
        if (this != &other) {
            reset();
            handle_ = other.handle_;
            other.handle_ = nullptr;
        }
        return *this;
    }
    ~Handle() { reset(); }
    HANDLE get() const { return handle_; }
    void reset() {
        if (handle_ != nullptr) {
            CloseHandle(handle_);
            handle_ = nullptr;
        }
    }

private:
    HANDLE handle_ = nullptr;
};

// Creates a pipe whose child end can be inherited but whose parent end cannot.
bool makePipe(Handle& readEnd, Handle& writeEnd, bool parentReads) {
    SECURITY_ATTRIBUTES attributes{};
    attributes.nLength = sizeof(attributes);
    attributes.bInheritHandle = TRUE;
    HANDLE read = nullptr;
    HANDLE write = nullptr;
    if (!CreatePipe(&read, &write, &attributes, 0)) {
        return false;
    }
    readEnd = Handle(read);
    writeEnd = Handle(write);
    HANDLE parentEnd = parentReads ? read : write;
    return SetHandleInformation(parentEnd, HANDLE_FLAG_INHERIT, 0) != 0;
}

void readAll(HANDLE handle, std::string& into) {
    char buffer[4096];
    DWORD count = 0;
    while (ReadFile(handle, buffer, sizeof(buffer), &count, nullptr) && count > 0) {
        into.append(buffer, count);
    }
}

} // namespace

std::optional<int> runProcess(const std::vector<std::string>& arguments) {
    if (arguments.empty()) {
        return std::nullopt;
    }
    std::string commandLine = windowsCommandLine(arguments);
    STARTUPINFOA startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    // CreateProcessA may modify the command line buffer, so it must be writable.
    if (!CreateProcessA(nullptr, commandLine.data(), nullptr, nullptr, TRUE, 0, nullptr, nullptr,
                        &startup, &process)) {
        return std::nullopt;
    }
    Handle processHandle(process.hProcess);
    Handle threadHandle(process.hThread);
    WaitForSingleObject(processHandle.get(), INFINITE);
    DWORD exitCode = 0;
    GetExitCodeProcess(processHandle.get(), &exitCode);
    return static_cast<int>(exitCode);
}

std::optional<ProcessResult> runProcessCaptured(const std::vector<std::string>& arguments,
                                                const CaptureOptions& options) {
    if (arguments.empty()) {
        return std::nullopt;
    }
    Handle inRead, inWrite, outRead, outWrite, errRead, errWrite;
    if (!makePipe(inRead, inWrite, false) || !makePipe(outRead, outWrite, true) ||
        !makePipe(errRead, errWrite, true)) {
        return std::nullopt;
    }

    std::string commandLine = windowsCommandLine(arguments);
    STARTUPINFOA startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = inRead.get();
    startup.hStdOutput = outWrite.get();
    startup.hStdError = errWrite.get();
    PROCESS_INFORMATION process{};
    if (!CreateProcessA(nullptr, commandLine.data(), nullptr, nullptr, TRUE, 0, nullptr, nullptr,
                        &startup, &process)) {
        return std::nullopt;
    }
    Handle processHandle(process.hProcess);
    Handle threadHandle(process.hThread);
    // The parent must close its copies of the child's ends, or reading never sees the end.
    inRead.reset();
    outWrite.reset();
    errWrite.reset();

    ProcessResult result;
    std::thread outReader([&] { readAll(outRead.get(), result.standardOutput); });
    std::thread errReader([&] { readAll(errRead.get(), result.standardError); });
    std::thread writer([&] {
        const std::string& data = options.standardInput;
        std::size_t offset = 0;
        while (offset < data.size()) {
            DWORD written = 0;
            if (!WriteFile(inWrite.get(), data.data() + offset,
                           static_cast<DWORD>(data.size() - offset), &written, nullptr) ||
                written == 0) {
                break;
            }
            offset += written;
        }
        inWrite.reset();
    });

    DWORD waitMilliseconds =
        options.timeoutSeconds > 0 ? static_cast<DWORD>(options.timeoutSeconds * 1000) : INFINITE;
    if (WaitForSingleObject(processHandle.get(), waitMilliseconds) == WAIT_TIMEOUT) {
        result.timedOut = true;
        TerminateProcess(processHandle.get(), 1);
        WaitForSingleObject(processHandle.get(), INFINITE);
    }
    outReader.join();
    errReader.join();
    writer.join();

    DWORD exitCode = 0;
    GetExitCodeProcess(processHandle.get(), &exitCode);
    result.exitCode = static_cast<int>(exitCode);
    return result;
}

#else

namespace {

// Owns a file descriptor.
class Fd {
public:
    Fd() = default;
    explicit Fd(int fd) : fd_(fd) {}
    Fd(const Fd&) = delete;
    Fd& operator=(const Fd&) = delete;
    Fd(Fd&& other) noexcept : fd_(other.fd_) { other.fd_ = -1; }
    Fd& operator=(Fd&& other) noexcept {
        if (this != &other) {
            reset();
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }
    ~Fd() { reset(); }
    int get() const { return fd_; }
    void reset() {
        if (fd_ >= 0) {
            close(fd_);
            fd_ = -1;
        }
    }

private:
    int fd_ = -1;
};

bool makePipe(Fd& readEnd, Fd& writeEnd) {
    int fds[2];
    if (pipe(fds) != 0) {
        return false;
    }
    readEnd = Fd(fds[0]);
    writeEnd = Fd(fds[1]);
    return true;
}

std::vector<char*> toArgv(const std::vector<std::string>& arguments) {
    std::vector<char*> argv;
    argv.reserve(arguments.size() + 1);
    for (const std::string& argument : arguments) {
        argv.push_back(const_cast<char*>(argument.c_str()));
    }
    argv.push_back(nullptr);
    return argv;
}

int exitCodeFromStatus(int status) {
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    // Killed by a signal: report it the way shells do.
    return 128 + (WIFSIGNALED(status) ? WTERMSIG(status) : 0);
}

using Clock = std::chrono::steady_clock;

} // namespace

std::optional<int> runProcess(const std::vector<std::string>& arguments) {
    if (arguments.empty()) {
        return std::nullopt;
    }
    std::vector<char*> argv = toArgv(arguments);
    pid_t pid = 0;
    if (posix_spawnp(&pid, argv[0], nullptr, nullptr, argv.data(), environ) != 0) {
        return std::nullopt;
    }
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        return std::nullopt;
    }
    return exitCodeFromStatus(status);
}

std::optional<ProcessResult> runProcessCaptured(const std::vector<std::string>& arguments,
                                                const CaptureOptions& options) {
    if (arguments.empty()) {
        return std::nullopt;
    }
    Fd inRead, inWrite, outRead, outWrite, errRead, errWrite;
    if (!makePipe(inRead, inWrite) || !makePipe(outRead, outWrite) ||
        !makePipe(errRead, errWrite)) {
        return std::nullopt;
    }

    posix_spawn_file_actions_t actions;
    if (posix_spawn_file_actions_init(&actions) != 0) {
        return std::nullopt;
    }
    posix_spawn_file_actions_adddup2(&actions, inRead.get(), 0);
    posix_spawn_file_actions_adddup2(&actions, outWrite.get(), 1);
    posix_spawn_file_actions_adddup2(&actions, errWrite.get(), 2);
    for (const Fd* fd : {&inRead, &inWrite, &outRead, &outWrite, &errRead, &errWrite}) {
        posix_spawn_file_actions_addclose(&actions, fd->get());
    }
    std::vector<char*> argv = toArgv(arguments);
    pid_t pid = 0;
    int spawnResult = posix_spawnp(&pid, argv[0], &actions, nullptr, argv.data(), environ);
    posix_spawn_file_actions_destroy(&actions);
    if (spawnResult != 0) {
        return std::nullopt;
    }
    // The parent must close its copies of the child's ends, or reading never sees the end.
    inRead.reset();
    outWrite.reset();
    errWrite.reset();

    // Feed standard input from a thread so a large input cannot block reading the output.
    std::thread writer;
    if (options.standardInput.empty()) {
        inWrite.reset();
    } else {
        writer = std::thread([&inWrite, &options] {
        // A child that exits without reading its input would otherwise kill us with
        // SIGPIPE; the write must fail with EPIPE instead. macOS delivers the signal to
        // the whole process, so it needs a per-descriptor flag. Elsewhere the signal goes
        // to the writing thread, where blocking it is enough.
#if defined(__APPLE__)
            fcntl(inWrite.get(), F_SETNOSIGPIPE, 1);
#else
            sigset_t blocked;
            sigemptyset(&blocked);
            sigaddset(&blocked, SIGPIPE);
            pthread_sigmask(SIG_BLOCK, &blocked, nullptr);
#endif
            const std::string& data = options.standardInput;
            std::size_t offset = 0;
            while (offset < data.size()) {
                ssize_t written = write(inWrite.get(), data.data() + offset, data.size() - offset);
                if (written < 0 && errno == EINTR) {
                    continue;
                }
                if (written <= 0) {
                    break;
                }
                offset += static_cast<std::size_t>(written);
            }
            inWrite.reset();
        });
    }

    ProcessResult result;
    const bool hasDeadline = options.timeoutSeconds > 0;
    const Clock::time_point deadline =
        Clock::now() + std::chrono::duration_cast<Clock::duration>(
                           std::chrono::duration<double>(hasDeadline ? options.timeoutSeconds : 0));
    auto remainingMilliseconds = [&]() -> long long {
        return std::chrono::duration_cast<std::chrono::milliseconds>(deadline - Clock::now())
            .count();
    };

    pollfd fds[2] = {{outRead.get(), POLLIN, 0}, {errRead.get(), POLLIN, 0}};
    std::string* targets[2] = {&result.standardOutput, &result.standardError};
    while (fds[0].fd >= 0 || fds[1].fd >= 0) {
        int waitMilliseconds = -1;
        if (hasDeadline) {
            long long remaining = remainingMilliseconds();
            if (remaining <= 0) {
                result.timedOut = true;
                break;
            }
            waitMilliseconds = static_cast<int>(std::min<long long>(remaining, 60'000));
        }
        if (poll(fds, 2, waitMilliseconds) < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        for (int i = 0; i < 2; ++i) {
            if (fds[i].fd < 0 || (fds[i].revents & (POLLIN | POLLHUP | POLLERR)) == 0) {
                continue;
            }
            char buffer[4096];
            ssize_t count = read(fds[i].fd, buffer, sizeof(buffer));
            if (count > 0) {
                targets[i]->append(buffer, static_cast<std::size_t>(count));
            } else if (count == 0 || (errno != EINTR && errno != EAGAIN)) {
                fds[i].fd = -1; // poll ignores negative descriptors
            }
        }
    }

    // The output is finished (or we gave up). Wait for the child itself, still honouring the
    // deadline, because a child can close its output and keep running.
    int status = 0;
    if (result.timedOut) {
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
    } else {
        for (;;) {
            pid_t done = waitpid(pid, &status, WNOHANG);
            if (done == pid) {
                break;
            }
            if (done < 0 && errno != EINTR) {
                break;
            }
            if (hasDeadline && remainingMilliseconds() <= 0) {
                result.timedOut = true;
                kill(pid, SIGKILL);
                waitpid(pid, &status, 0);
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }
    // Closing the read ends first guarantees the writer cannot stay blocked.
    outRead.reset();
    errRead.reset();
    if (writer.joinable()) {
        writer.join();
    }
    result.exitCode = exitCodeFromStatus(status);
    return result;
}

#endif

} // namespace zack::driver
