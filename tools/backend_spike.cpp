// Phase 0 spike (plan items 0.4 and 0.5): builds a tiny program through the Backend,
// emits an object file and LLVM IR text, links it with the C runtime, and leaves an
// executable behind. It proves the toolchain end to end; it is not part of the compiler.
//
// Usage: backend_spike <mode> <output-executable> <runtime-library>
//
// Modes:
//   exit42   int main() { return 40 + 2; }
//   print42  int main() { zack_print_i32(40 + 2); return 0; }
//   panic    int main() { zack_panic("boom", "spike.zl", 7); }

#include "codegen/backend.hpp"
#include "driver/linker.hpp"

#include <cstdio>
#include <string>

namespace {

using zack::codegen::Backend;
using zack::codegen::Function;
using zack::codegen::Type;

constexpr int kFailure = 1;

bool buildProgram(Backend& backend, const std::string& mode) {
    Function mainFunction = backend.declareFunction("main", Type::I32, {});

    if (mode == "exit42") {
        backend.beginBody(mainFunction);
        backend.ret(backend.add(backend.constI32(40), backend.constI32(2)));
        return true;
    }
    if (mode == "print42") {
        Function print = backend.declareFunction("zack_print_i32", Type::Void, {Type::I32});
        backend.beginBody(mainFunction);
        backend.call(print, {backend.add(backend.constI32(40), backend.constI32(2))});
        backend.ret(backend.constI32(0));
        return true;
    }
    if (mode == "panic") {
        Function panic =
            backend.declareFunction("zack_panic", Type::Void, {Type::Ptr, Type::Ptr, Type::I32});
        backend.beginBody(mainFunction);
        backend.call(panic, {backend.constString("boom"), backend.constString("spike.zl"),
                             backend.constI32(7)});
        backend.unreachable();
        return true;
    }
    return false;
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 4) {
        std::fputs("usage: backend_spike <exit42|print42|panic> <output-executable> "
                   "<runtime-library>\n",
                   stderr);
        return kFailure;
    }
    const std::string mode = argv[1];
    const std::string output = argv[2];
    const std::string runtimeLibrary = argv[3];

    Backend backend("spike_" + mode);
    if (!buildProgram(backend, mode)) {
        std::fprintf(stderr, "backend_spike: unknown mode '%s'\n", mode.c_str());
        return kFailure;
    }
    if (auto error = backend.verify()) {
        std::fprintf(stderr, "backend_spike: module verification failed: %s\n", error->c_str());
        return kFailure;
    }
    if (auto error = backend.runPasses("mem2reg")) {
        std::fprintf(stderr, "backend_spike: running passes failed: %s\n", error->c_str());
        return kFailure;
    }
    if (auto error = backend.emitIrFile(output + ".ll")) {
        std::fprintf(stderr, "backend_spike: writing IR failed: %s\n", error->c_str());
        return kFailure;
    }
    const std::string objectFile = output + ".o";
    if (auto error = backend.emitObjectFile(objectFile)) {
        std::fprintf(stderr, "backend_spike: writing the object file failed: %s\n", error->c_str());
        return kFailure;
    }
    if (auto error = zack::driver::linkExecutable(objectFile, runtimeLibrary, output)) {
        std::fprintf(stderr, "backend_spike: %s\n", error->c_str());
        return kFailure;
    }
    return 0;
}
