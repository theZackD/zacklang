#include "codegen/backend.hpp"

#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/Error.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Transforms/PassBuilder.h>

#include <limits>
#include <mutex>
#include <string>

namespace zack::codegen {

namespace {

constexpr std::uint32_t kInvalidId = std::numeric_limits<std::uint32_t>::max();

// LLVM returns messages as malloc'd strings that must be released with LLVMDisposeMessage.
std::string takeMessage(char* message) {
    std::string text = message != nullptr ? message : "";
    LLVMDisposeMessage(message);
    return text;
}

void initializeLlvmOnce() {
    static std::once_flag flag;
    std::call_once(flag, [] {
        LLVMInitializeNativeTarget();
        LLVMInitializeNativeAsmPrinter();
    });
}

struct FunctionInfo {
    LLVMValueRef function;
    LLVMTypeRef type;
    bool returnsVoid;
};

} // namespace

std::string llvmVersionString() {
    unsigned major = 0;
    unsigned minor = 0;
    unsigned patch = 0;
    LLVMGetVersion(&major, &minor, &patch);
    return "LLVM " + std::to_string(major) + "." + std::to_string(minor) + "." +
           std::to_string(patch);
}

unsigned llvmMajorVersion() {
    unsigned major = 0;
    unsigned minor = 0;
    unsigned patch = 0;
    LLVMGetVersion(&major, &minor, &patch);
    return major;
}

struct Backend::Impl {
    LLVMContextRef context = nullptr;
    LLVMModuleRef module = nullptr;
    LLVMBuilderRef builder = nullptr;
    LLVMTargetMachineRef targetMachine = nullptr;
    std::vector<FunctionInfo> functions;
    std::vector<LLVMValueRef> values;

    LLVMTypeRef lower(Type type) const {
        switch (type) {
        case Type::Void:
            return LLVMVoidTypeInContext(context);
        case Type::I32:
            return LLVMInt32TypeInContext(context);
        case Type::Ptr:
            return LLVMPointerTypeInContext(context, 0);
        }
        return nullptr;
    }

    Value record(LLVMValueRef value) {
        values.push_back(value);
        return Value{static_cast<std::uint32_t>(values.size() - 1)};
    }
};

Backend::Backend(const std::string& moduleName) : impl_(std::make_unique<Impl>()) {
    initializeLlvmOnce();
    impl_->context = LLVMContextCreate();
    impl_->module = LLVMModuleCreateWithNameInContext(moduleName.c_str(), impl_->context);
    impl_->builder = LLVMCreateBuilderInContext(impl_->context);

    char* triple = LLVMGetDefaultTargetTriple();
    LLVMTargetRef target = nullptr;
    char* error = nullptr;
    if (LLVMGetTargetFromTriple(triple, &target, &error) == 0) {
        impl_->targetMachine =
            LLVMCreateTargetMachine(target, triple, "generic", "", LLVMCodeGenLevelDefault,
                                    LLVMRelocPIC, LLVMCodeModelDefault);
        LLVMSetTarget(impl_->module, triple);
        LLVMTargetDataRef layout = LLVMCreateTargetDataLayout(impl_->targetMachine);
        LLVMSetModuleDataLayout(impl_->module, layout);
        LLVMDisposeTargetData(layout);
    } else {
        LLVMDisposeMessage(error);
    }
    LLVMDisposeMessage(triple);
}

Backend::~Backend() {
    if (impl_->targetMachine != nullptr) {
        LLVMDisposeTargetMachine(impl_->targetMachine);
    }
    LLVMDisposeBuilder(impl_->builder);
    LLVMDisposeModule(impl_->module);
    LLVMContextDispose(impl_->context);
}

Function Backend::declareFunction(const std::string& name, Type returnType,
                                  const std::vector<Type>& parameterTypes) {
    std::vector<LLVMTypeRef> params;
    params.reserve(parameterTypes.size());
    for (Type type : parameterTypes) {
        params.push_back(impl_->lower(type));
    }
    LLVMTypeRef functionType = LLVMFunctionType(impl_->lower(returnType), params.data(),
                                                static_cast<unsigned>(params.size()), 0);
    LLVMValueRef function = LLVMAddFunction(impl_->module, name.c_str(), functionType);
    impl_->functions.push_back({function, functionType, returnType == Type::Void});
    return Function{static_cast<std::uint32_t>(impl_->functions.size() - 1)};
}

void Backend::beginBody(Function function) {
    LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(
        impl_->context, impl_->functions[function.id].function, "entry");
    LLVMPositionBuilderAtEnd(impl_->builder, entry);
}

Value Backend::constI32(std::int32_t value) {
    // The C API takes the value as unsigned 64-bit plus a sign-extension flag.
    return impl_->record(
        LLVMConstInt(LLVMInt32TypeInContext(impl_->context),
                     static_cast<unsigned long long>(static_cast<long long>(value)), 1));
}

Value Backend::constString(const std::string& text) {
    return impl_->record(LLVMBuildGlobalStringPtr(impl_->builder, text.c_str(), "str"));
}

Value Backend::add(Value lhs, Value rhs) {
    return impl_->record(
        LLVMBuildAdd(impl_->builder, impl_->values[lhs.id], impl_->values[rhs.id], "add"));
}

Value Backend::call(Function callee, const std::vector<Value>& arguments) {
    const FunctionInfo& info = impl_->functions[callee.id];
    std::vector<LLVMValueRef> args;
    args.reserve(arguments.size());
    for (Value argument : arguments) {
        args.push_back(impl_->values[argument.id]);
    }
    // A void value cannot be given a name, so void calls get an empty one.
    LLVMValueRef result =
        LLVMBuildCall2(impl_->builder, info.type, info.function, args.data(),
                       static_cast<unsigned>(args.size()), info.returnsVoid ? "" : "call");
    if (info.returnsVoid) {
        return Value{kInvalidId};
    }
    return impl_->record(result);
}

void Backend::ret(Value value) {
    LLVMBuildRet(impl_->builder, impl_->values[value.id]);
}

void Backend::retVoid() {
    LLVMBuildRetVoid(impl_->builder);
}

void Backend::unreachable() {
    LLVMBuildUnreachable(impl_->builder);
}

std::optional<std::string> Backend::verify() {
    char* error = nullptr;
    if (LLVMVerifyModule(impl_->module, LLVMReturnStatusAction, &error) != 0) {
        return takeMessage(error);
    }
    LLVMDisposeMessage(error);
    return std::nullopt;
}

std::optional<std::string> Backend::runPasses(const std::string& pipeline) {
    LLVMPassBuilderOptionsRef options = LLVMCreatePassBuilderOptions();
    LLVMErrorRef error =
        LLVMRunPasses(impl_->module, pipeline.c_str(), impl_->targetMachine, options);
    LLVMDisposePassBuilderOptions(options);
    if (error != nullptr) {
        return takeMessage(LLVMGetErrorMessage(error));
    }
    return std::nullopt;
}

std::string Backend::irText() const {
    return takeMessage(LLVMPrintModuleToString(impl_->module));
}

std::optional<std::string> Backend::emitIrFile(const std::string& path) const {
    char* error = nullptr;
    if (LLVMPrintModuleToFile(impl_->module, path.c_str(), &error) != 0) {
        return takeMessage(error);
    }
    return std::nullopt;
}

std::optional<std::string> Backend::emitObjectFile(const std::string& path) {
    if (impl_->targetMachine == nullptr) {
        return "no target machine is available for the host triple";
    }
    char* error = nullptr;
    // The C API takes a non-const char* for the path.
    std::string mutablePath = path;
    if (LLVMTargetMachineEmitToFile(impl_->targetMachine, impl_->module, mutablePath.data(),
                                    LLVMObjectFile, &error) != 0) {
        return takeMessage(error);
    }
    return std::nullopt;
}

} // namespace zack::codegen
