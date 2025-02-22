// src/zir_c_api.cpp
#include "../include/zir_c_api.h"
#include "../include/zir_basic_block.hpp"
#include "../include/zir_value.hpp"
#include "../include/zir_function.hpp"
#include "../include/zir_type.hpp"
#include "../include/zir_builder.hpp"
#include <memory>
#include <cstdio>

using namespace zir;

extern "C"
{

    ZIRBuilder *zir_create_builder()
    {
        return reinterpret_cast<ZIRBuilder *>(new ZIRBuilderImpl());
    }

    void zir_destroy_builder(ZIRBuilder *builder)
    {
        if (builder)
        {
            delete reinterpret_cast<ZIRBuilderImpl *>(builder);
        }
    }

    const char *zir_get_version(ZIRBuilder *builder)
    {
        if (!builder)
        {
            return nullptr;
        }
        auto *cpp_builder = reinterpret_cast<ZIRBuilderImpl *>(builder);
        static std::string version = cpp_builder->getVersion();
        return version.c_str();
    }

    // Type creation functions
    ZIRTypeHandle zir_create_i32_type()
    {
        auto type = std::make_shared<zir::ZIRIntegerType>(zir::ZIRIntegerType::Width::Int32);
        auto *handle = new std::shared_ptr<zir::ZIRType>(std::static_pointer_cast<zir::ZIRType>(type));
        return reinterpret_cast<ZIRTypeHandle>(handle);
    }

    ZIRTypeHandle zir_create_i64_type()
    {
        auto type = std::make_shared<zir::ZIRIntegerType>(zir::ZIRIntegerType::Width::Int64);
        auto *handle = new std::shared_ptr<zir::ZIRType>(std::static_pointer_cast<zir::ZIRType>(type));
        return reinterpret_cast<ZIRTypeHandle>(handle);
    }

    ZIRTypeHandle zir_create_f32_type()
    {
        auto type = std::make_shared<zir::ZIRFloatType>(zir::ZIRFloatType::Width::Float32);
        auto *handle = new std::shared_ptr<zir::ZIRType>(std::static_pointer_cast<zir::ZIRType>(type));
        return reinterpret_cast<ZIRTypeHandle>(handle);
    }

    ZIRTypeHandle zir_create_f64_type()
    {
        auto type = std::make_shared<zir::ZIRFloatType>(zir::ZIRFloatType::Width::Float64);
        auto *handle = new std::shared_ptr<zir::ZIRType>(std::static_pointer_cast<zir::ZIRType>(type));
        return reinterpret_cast<ZIRTypeHandle>(handle);
    }

    ZIRTypeHandle zir_create_bool_type()
    {
        auto type = std::make_shared<zir::ZIRBooleanType>();
        auto *handle = new std::shared_ptr<zir::ZIRType>(std::static_pointer_cast<zir::ZIRType>(type));
        return reinterpret_cast<ZIRTypeHandle>(handle);
    }

    ZIRTypeHandle zir_create_string_type()
    {
        auto type = std::make_shared<zir::ZIRStringType>();
        auto *handle = new std::shared_ptr<zir::ZIRType>(std::static_pointer_cast<zir::ZIRType>(type));
        return reinterpret_cast<ZIRTypeHandle>(handle);
    }

    // Literal creation functions
    ZIRValueHandle zir_create_integer_literal(ZIRTypeHandle type_handle, int64_t value)
    {
        if (!type_handle)
            return nullptr;
        auto *type_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRType> *>(type_handle);
        auto int_type = std::dynamic_pointer_cast<zir::ZIRIntegerType>(*type_ptr);
        if (!int_type)
            return nullptr;

        auto literal = std::make_shared<zir::ZIRIntegerLiteral>(int_type, value);
        auto *handle = new std::shared_ptr<zir::ZIRValue>(std::static_pointer_cast<zir::ZIRValue>(literal));
        return reinterpret_cast<ZIRValueHandle>(handle);
    }

    ZIRValueHandle zir_create_float_literal(ZIRTypeHandle type_handle, double value)
    {
        if (!type_handle)
            return nullptr;
        auto *type_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRType> *>(type_handle);
        auto float_type = std::dynamic_pointer_cast<zir::ZIRFloatType>(*type_ptr);
        if (!float_type)
            return nullptr;

        auto literal = std::make_shared<zir::ZIRFloatLiteral>(float_type, value);
        auto *handle = new std::shared_ptr<zir::ZIRValue>(std::static_pointer_cast<zir::ZIRValue>(literal));
        return reinterpret_cast<ZIRValueHandle>(handle);
    }

    ZIRValueHandle zir_create_bool_literal(ZIRTypeHandle type_handle, bool value)
    {
        if (!type_handle)
            return nullptr;
        auto *type_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRType> *>(type_handle);
        auto bool_type = std::dynamic_pointer_cast<zir::ZIRBooleanType>(*type_ptr);
        if (!bool_type)
            return nullptr;

        auto literal = std::make_shared<zir::ZIRBooleanLiteral>(bool_type, value);
        auto *handle = new std::shared_ptr<zir::ZIRValue>(std::static_pointer_cast<zir::ZIRValue>(literal));
        return reinterpret_cast<ZIRValueHandle>(handle);
    }

    ZIRValueHandle zir_create_string_literal(ZIRTypeHandle type_handle, const char *value)
    {
        if (!type_handle || !value)
            return nullptr;
        auto *type_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRType> *>(type_handle);
        auto string_type = std::dynamic_pointer_cast<zir::ZIRStringType>(*type_ptr);
        if (!string_type)
            return nullptr;

        auto literal = std::make_shared<zir::ZIRStringLiteral>(string_type, value);
        auto *handle = new std::shared_ptr<zir::ZIRValue>(std::static_pointer_cast<zir::ZIRValue>(literal));
        return reinterpret_cast<ZIRValueHandle>(handle);
    }

    // Type checking functions
    bool zir_is_integer_type(ZIRTypeHandle type_handle)
    {
        if (!type_handle)
            return false;
        auto *type_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRType> *>(type_handle);
        return std::dynamic_pointer_cast<zir::ZIRIntegerType>(*type_ptr) != nullptr;
    }

    bool zir_is_float_type(ZIRTypeHandle type_handle)
    {
        if (!type_handle)
            return false;
        auto *type_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRType> *>(type_handle);
        return std::dynamic_pointer_cast<zir::ZIRFloatType>(*type_ptr) != nullptr;
    }

    bool zir_is_bool_type(ZIRTypeHandle type_handle)
    {
        if (!type_handle)
            return false;
        auto *type_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRType> *>(type_handle);
        return std::dynamic_pointer_cast<zir::ZIRBooleanType>(*type_ptr) != nullptr;
    }

    bool zir_is_string_type(ZIRTypeHandle type_handle)
    {
        if (!type_handle)
            return false;
        auto *type_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRType> *>(type_handle);
        return std::dynamic_pointer_cast<zir::ZIRStringType>(*type_ptr) != nullptr;
    }

    // Value access functions
    int64_t zir_get_integer_value(ZIRValueHandle value_handle)
    {
        if (!value_handle)
            return 0;
        auto *value_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRValue> *>(value_handle);
        auto int_literal = std::dynamic_pointer_cast<zir::ZIRIntegerLiteral>(*value_ptr);
        return int_literal ? int_literal->getValue() : 0;
    }

    double zir_get_float_value(ZIRValueHandle value_handle)
    {
        if (!value_handle)
            return 0.0;
        auto *value_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRValue> *>(value_handle);
        auto float_literal = std::dynamic_pointer_cast<zir::ZIRFloatLiteral>(*value_ptr);
        return float_literal ? float_literal->getValue() : 0.0;
    }

    bool zir_get_bool_value(ZIRValueHandle value_handle)
    {
        if (!value_handle)
            return false;
        auto *value_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRValue> *>(value_handle);
        auto bool_literal = std::dynamic_pointer_cast<zir::ZIRBooleanLiteral>(*value_ptr);
        return bool_literal ? bool_literal->getValue() : false;
    }

    const char *zir_get_string_value(ZIRValueHandle value_handle)
    {
        if (!value_handle)
            return nullptr;
        auto *value_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRValue> *>(value_handle);
        auto string_literal = std::dynamic_pointer_cast<zir::ZIRStringLiteral>(*value_ptr);
        return string_literal ? string_literal->getValue().c_str() : nullptr;
    }

    // Cleanup functions
    void zir_destroy_type(ZIRTypeHandle type_handle)
    {
        if (type_handle)
        {
            auto *type_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRType> *>(type_handle);
            delete type_ptr;
        }
    }

    void zir_destroy_value(ZIRValueHandle value_handle)
    {
        if (value_handle)
        {
            auto *value_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRValue> *>(value_handle);
            delete value_ptr;
        }
    }

    // Basic block functions
    ZIRBasicBlockHandle zir_create_basic_block(const char *name)
    {
        if (!name)
            return nullptr;
        auto *block = new std::shared_ptr<ZIRBasicBlockImpl>(
            std::make_shared<ZIRBasicBlockImpl>(name));
        return reinterpret_cast<ZIRBasicBlockHandle>(block);
    }

    void zir_destroy_basic_block(ZIRBasicBlockHandle block)
    {
        if (block)
        {
            auto *block_ptr = reinterpret_cast<std::shared_ptr<ZIRBasicBlockImpl> *>(block);
            delete block_ptr;
        }
    }

    const char *zir_get_block_name(ZIRBasicBlockHandle block)
    {
        if (!block)
            return nullptr;
        auto *block_ptr = reinterpret_cast<std::shared_ptr<ZIRBasicBlockImpl> *>(block);
        static thread_local std::string current_name;
        current_name = (*block_ptr)->getName();
        return current_name.c_str();
    }

    void zir_set_block_name(ZIRBasicBlockHandle block, const char *name)
    {
        if (!block || !name)
            return;
        auto *block_ptr = reinterpret_cast<std::shared_ptr<ZIRBasicBlockImpl> *>(block);
        (*block_ptr)->setName(name);
    }

    uint64_t zir_get_block_id(ZIRBasicBlockHandle block)
    {
        if (!block)
            return UINT64_MAX; // Return max value to indicate invalid block
        auto *block_ptr = reinterpret_cast<std::shared_ptr<ZIRBasicBlockImpl> *>(block);
        return (*block_ptr)->getId();
    }

    ZIRFunctionHandle zir_create_function(const char *name)
    {
        if (!name)
        {
            return nullptr;
        }
        auto *function = new std::shared_ptr<zir::ZIRFunctionImpl>(
            std::make_shared<zir::ZIRFunctionImpl>(name));
        return reinterpret_cast<ZIRFunctionHandle>(function);
    }

    void zir_destroy_function(ZIRFunctionHandle function)
    {
        if (function)
        {
            auto *function_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRFunctionImpl> *>(function);
            delete function_ptr;
        }
    }

    const char *zir_get_function_name(ZIRFunctionHandle function)
    {
        if (!function)
        {
            return nullptr;
        }
        auto *function_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRFunctionImpl> *>(function);
        thread_local static std::string current_name;
        current_name = (*function_ptr)->getName();
        return current_name.c_str();
    }

    void zir_set_function_name(ZIRFunctionHandle function, const char *name)
    {
        if (!function || !name)
        {
            return;
        }
        auto *function_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRFunctionImpl> *>(function);
        (*function_ptr)->setName(name);
    }

    void zir_set_block_parent(ZIRBasicBlockHandle block, ZIRFunctionHandle function)
    {
        if (!block)
        {
            return;
        }
        auto *block_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRBasicBlockImpl> *>(block);
        printf("Setting parent: function=%p\n", function);
        if (function)
        {
            auto *function_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRFunctionImpl> *>(function);
            (*block_ptr)->setParentFunction(static_cast<void *>(function));
            (*function_ptr)->addBlock(*block_ptr);
        }
        else
        {
            if (auto parent = (*block_ptr)->getParentFunction())
            {
                auto *parent_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRFunctionImpl> *>(parent);
                (*parent_ptr)->removeBlock(*block_ptr);
                (*block_ptr)->setParentFunction(nullptr);
            }
        }
    }

    ZIRFunctionHandle zir_get_block_parent(ZIRBasicBlockHandle block)
    {
        if (!block)
        {
            return nullptr;
        }
        auto *cpp_block = reinterpret_cast<std::shared_ptr<zir::ZIRBasicBlockImpl> *>(block);
        void *parent = (*cpp_block)->getParentFunction();
        printf("Getting parent: stored=%p\n", parent);
        return static_cast<ZIRFunctionHandle>(parent);
    }

    void zir_function_add_block(ZIRFunctionHandle function, ZIRBasicBlockHandle block)
    {
        if (!function || !block)
        {
            return;
        }
        auto *function_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRFunctionImpl> *>(function);
        auto *block_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRBasicBlockImpl> *>(block);
        try
        {
            (*block_ptr)->setParentFunction(static_cast<void *>(function));
            (*function_ptr)->addBlock(*block_ptr);
        }
        catch (const std::exception &)
        {
            // Ignore any exceptions
        }
    }

    void zir_function_remove_block(ZIRFunctionHandle function, ZIRBasicBlockHandle block)
    {
        if (!function || !block)
        {
            return;
        }
        auto *function_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRFunctionImpl> *>(function);
        auto *block_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRBasicBlockImpl> *>(block);
        (*function_ptr)->removeBlock(*block_ptr);
        (*block_ptr)->setParentFunction(nullptr);
    }

    size_t zir_function_get_block_count(ZIRFunctionHandle function)
    {
        if (!function)
        {
            return 0;
        }
        auto *function_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRFunctionImpl> *>(function);
        return (*function_ptr)->getBlockCount();
    }

    ZIRBasicBlockHandle zir_function_get_block(ZIRFunctionHandle function, size_t index)
    {
        if (!function)
        {
            return nullptr;
        }
        auto *function_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRFunctionImpl> *>(function);
        auto block = (*function_ptr)->getBlock(index);
        if (!block)
        {
            return nullptr;
        }
        auto *block_ptr = new std::shared_ptr<zir::ZIRBasicBlockImpl>(block);
        return reinterpret_cast<ZIRBasicBlockHandle>(block_ptr);
    }
}