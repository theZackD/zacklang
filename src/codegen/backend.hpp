#ifndef ZACK_CODEGEN_BACKEND_HPP
#define ZACK_CODEGEN_BACKEND_HPP

// The Backend is the only part of the compiler that talks to LLVM, and it does so through
// the LLVM C API only (decisions D2 and D3). This header deliberately includes no LLVM
// header: callers see small opaque handles, not LLVM types.

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace zack::codegen {

enum class Type { Void, I32, Ptr };

// Handles are indices into tables owned by the Backend. They are only valid for the
// Backend that created them.
struct Value {
    std::uint32_t id;
};
struct Function {
    std::uint32_t id;
};

// "LLVM 20.1.6" style text for the LLVM library this program was linked with.
std::string llvmVersionString();
unsigned llvmMajorVersion();

// Error convention: functions that can fail return the error message, or std::nullopt on
// success. The Backend never throws and never exits the process.
class Backend {
public:
    explicit Backend(const std::string& moduleName);
    ~Backend();
    Backend(const Backend&) = delete;
    Backend& operator=(const Backend&) = delete;

    // Declares a function with external linkage. A function becomes a definition once
    // beginBody() is called on it.
    Function declareFunction(const std::string& name, Type returnType,
                             const std::vector<Type>& parameterTypes);

    // Creates the entry block of `function` and positions the builder in it.
    void beginBody(Function function);

    Value constI32(std::int32_t value);
    // A pointer to a NUL-terminated copy of `text` stored in the module.
    Value constString(const std::string& text);
    Value add(Value lhs, Value rhs);
    // Returns an invalid Value (id == UINT32_MAX) when the callee returns void.
    Value call(Function callee, const std::vector<Value>& arguments);
    void ret(Value value);
    void retVoid();
    // Terminates the current block with `unreachable`, for calls that never return.
    void unreachable();

    // Checks the module with LLVM's verifier. A failure is a bug in the compiler (an
    // internal error), never a user error.
    std::optional<std::string> verify();
    // Runs an LLVM new-pass-manager pipeline, for example "mem2reg" or "default<O2>".
    std::optional<std::string> runPasses(const std::string& pipeline);
    // The module as LLVM IR text (debugging aid, never a test oracle: decision D13).
    std::string irText() const;
    std::optional<std::string> emitIrFile(const std::string& path) const;
    // Writes an object file for the host target.
    std::optional<std::string> emitObjectFile(const std::string& path);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace zack::codegen

#endif // ZACK_CODEGEN_BACKEND_HPP
