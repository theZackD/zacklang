#include "codegen/backend.hpp"

#include <cstdio>
#include <cstring>

namespace {

constexpr int kExitOk = 0;
constexpr int kExitUsage = 2;

void printUsage(std::FILE* out) {
    std::fputs("Usage: zackc [options]\n"
               "\n"
               "Options:\n"
               "  --help      Show this help\n"
               "  --version   Show the compiler and LLVM versions\n",
               out);
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage(stderr);
        return kExitUsage;
    }
    const char* command = argv[1];
    if (std::strcmp(command, "--help") == 0) {
        printUsage(stdout);
        return kExitOk;
    }
    if (std::strcmp(command, "--version") == 0) {
        std::printf("zackc %s\n%s\n", ZACK_VERSION, zack::codegen::llvmVersionString().c_str());
        return kExitOk;
    }
    std::fprintf(stderr, "zackc: unknown command '%s'\n\n", command);
    printUsage(stderr);
    return kExitUsage;
}
