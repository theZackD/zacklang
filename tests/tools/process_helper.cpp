// A tiny program with predictable behaviour, used by the process-launcher tests.
//
//   process_helper exit <n>        exit with code n
//   process_helper echo-stdin      copy standard input to standard output
//   process_helper out-err         write "out" to standard output and "err" to standard error
//   process_helper flood <bytes>   write that many bytes to standard output and to standard error
//   process_helper sleep <secs>    wait, then exit with code 0

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>

int main(int argc, char** argv) {
    if (argc < 2) {
        return 64;
    }
    const std::string mode = argv[1];
    if (mode == "exit" && argc == 3) {
        return std::atoi(argv[2]);
    }
    if (mode == "echo-stdin") {
        char buffer[4096];
        std::size_t count = 0;
        while ((count = std::fread(buffer, 1, sizeof(buffer), stdin)) > 0) {
            std::fwrite(buffer, 1, count, stdout);
        }
        return 0;
    }
    if (mode == "out-err") {
        std::fputs("out", stdout);
        std::fputs("err", stderr);
        return 0;
    }
    if (mode == "flood" && argc == 3) {
        // Far more than a pipe buffer holds, on both streams, to catch reading deadlocks.
        const long total = std::atol(argv[2]);
        const std::string chunk(1000, 'x');
        for (long written = 0; written < total; written += static_cast<long>(chunk.size())) {
            std::fwrite(chunk.data(), 1, chunk.size(), stdout);
            std::fwrite(chunk.data(), 1, chunk.size(), stderr);
        }
        return 0;
    }
    if (mode == "sleep" && argc == 3) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(static_cast<long long>(std::atof(argv[2]) * 1000)));
        return 0;
    }
    return 64;
}
