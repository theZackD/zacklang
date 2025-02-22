// src/zir_c_api.cpp
#include "../include/zir_c_api.h"
#include "../include/zir_basic_block.hpp"
#include "../include/zir_value.hpp"
#include "../include/zir_function.hpp"
#include "../include/zir_type.hpp"
#include "../include/zir_builder.hpp"
#include "../include/zir_instruction_impl.hpp"
#include "../include/zir_arithmetic.hpp"
#include "../include/zir_comparison.hpp"
#include "../include/zir_logical.hpp"
#include "../include/zir_control_flow.hpp"
#include "../include/zir_instruction.hpp"
#include "../include/zir_value_impl.hpp"
#include <memory>
#include <cstdio>
#include <iostream>
#include <string>

using namespace zir;

// Helper functions for handle conversion
static std::shared_ptr<ZIRFunctionImpl> *handle_to_function(zir_function_handle handle)
{
    return reinterpret_cast<std::shared_ptr<ZIRFunctionImpl> *>(handle);
}

static std::shared_ptr<ZIRBasicBlockImpl> *handle_to_block(zir_block_handle handle)
{
    return reinterpret_cast<std::shared_ptr<ZIRBasicBlockImpl> *>(handle);
}

static std::shared_ptr<ZIRInstructionImpl> *handle_to_instruction(zir_instruction_handle handle)
{
    return reinterpret_cast<std::shared_ptr<ZIRInstructionImpl> *>(handle);
}

static std::shared_ptr<ZIRValueImpl> *handle_to_value(zir_value_handle handle)
{
    return reinterpret_cast<std::shared_ptr<ZIRValueImpl> *>(handle);
}

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
    zir_type_handle zir_create_i32_type()
    {
        auto type = std::make_shared<zir::ZIRIntegerType>(zir::ZIRIntegerType::Width::Int32);
        auto *handle = new std::shared_ptr<zir::ZIRType>(std::static_pointer_cast<zir::ZIRType>(type));
        return reinterpret_cast<zir_type_handle>(handle);
    }

    zir_type_handle zir_create_i64_type()
    {
        auto type = std::make_shared<zir::ZIRIntegerType>(zir::ZIRIntegerType::Width::Int64);
        auto *handle = new std::shared_ptr<zir::ZIRType>(std::static_pointer_cast<zir::ZIRType>(type));
        return reinterpret_cast<zir_type_handle>(handle);
    }

    zir_type_handle zir_create_f32_type()
    {
        auto type = std::make_shared<zir::ZIRFloatType>(zir::ZIRFloatType::Width::Float32);
        auto *handle = new std::shared_ptr<zir::ZIRType>(std::static_pointer_cast<zir::ZIRType>(type));
        return reinterpret_cast<zir_type_handle>(handle);
    }

    zir_type_handle zir_create_f64_type()
    {
        auto type = std::make_shared<zir::ZIRFloatType>(zir::ZIRFloatType::Width::Float64);
        auto *handle = new std::shared_ptr<zir::ZIRType>(std::static_pointer_cast<zir::ZIRType>(type));
        return reinterpret_cast<zir_type_handle>(handle);
    }

    zir_type_handle zir_create_bool_type()
    {
        auto type = std::make_shared<zir::ZIRBooleanType>();
        auto *handle = new std::shared_ptr<zir::ZIRType>(std::static_pointer_cast<zir::ZIRType>(type));
        return reinterpret_cast<zir_type_handle>(handle);
    }

    zir_type_handle zir_create_string_type()
    {
        auto type = std::make_shared<zir::ZIRStringType>();
        auto *handle = new std::shared_ptr<zir::ZIRType>(std::static_pointer_cast<zir::ZIRType>(type));
        return reinterpret_cast<zir_type_handle>(handle);
    }

    // Literal creation functions
    zir_value_handle zir_create_integer_literal(zir_type_handle type, int64_t value)
    {
        if (!type)
        {
            return nullptr;
        }
        auto *type_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRType> *>(type);
        auto int_type = std::dynamic_pointer_cast<zir::ZIRIntegerType>(*type_ptr);
        if (!int_type)
        {
            return nullptr;
        }
        auto literal = std::make_shared<zir::ZIRIntegerLiteral>(int_type, value);
        auto *handle = new std::shared_ptr<zir::ZIRValue>(std::static_pointer_cast<zir::ZIRValue>(literal));
        return reinterpret_cast<zir_value_handle>(handle);
    }

    zir_value_handle zir_create_float_literal(zir_type_handle type, double value)
    {
        if (!type)
        {
            return nullptr;
        }
        auto *type_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRType> *>(type);
        auto float_type = std::dynamic_pointer_cast<zir::ZIRFloatType>(*type_ptr);
        if (!float_type)
        {
            return nullptr;
        }
        auto literal = std::make_shared<zir::ZIRFloatLiteral>(float_type, value);
        auto *handle = new std::shared_ptr<zir::ZIRValue>(std::static_pointer_cast<zir::ZIRValue>(literal));
        return reinterpret_cast<zir_value_handle>(handle);
    }

    zir_value_handle zir_create_bool_literal(zir_type_handle type, bool value)
    {
        if (!type)
        {
            return nullptr;
        }
        auto *type_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRType> *>(type);
        auto bool_type = std::dynamic_pointer_cast<zir::ZIRBooleanType>(*type_ptr);
        if (!bool_type)
        {
            return nullptr;
        }
        auto literal = std::make_shared<zir::ZIRBooleanLiteral>(bool_type, value);
        auto *handle = new std::shared_ptr<zir::ZIRValue>(std::static_pointer_cast<zir::ZIRValue>(literal));
        return reinterpret_cast<zir_value_handle>(handle);
    }

    zir_value_handle zir_create_string_literal(zir_type_handle type, const char *value)
    {
        if (!type || !value)
        {
            return nullptr;
        }
        auto *type_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRType> *>(type);
        auto string_type = std::dynamic_pointer_cast<zir::ZIRStringType>(*type_ptr);
        if (!string_type)
        {
            return nullptr;
        }
        auto literal = std::make_shared<zir::ZIRStringLiteral>(string_type, std::string(value));
        auto *handle = new std::shared_ptr<zir::ZIRValue>(std::static_pointer_cast<zir::ZIRValue>(literal));
        return reinterpret_cast<zir_value_handle>(handle);
    }

    // Type checking functions
    bool zir_is_integer_type(zir_type_handle type)
    {
        if (!type)
        {
            return false;
        }
        auto *type_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRType> *>(type);
        return std::dynamic_pointer_cast<zir::ZIRIntegerType>(*type_ptr) != nullptr;
    }

    bool zir_is_float_type(zir_type_handle type)
    {
        if (!type)
        {
            return false;
        }
        auto *type_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRType> *>(type);
        return std::dynamic_pointer_cast<zir::ZIRFloatType>(*type_ptr) != nullptr;
    }

    bool zir_is_bool_type(zir_type_handle type)
    {
        if (!type)
        {
            return false;
        }
        auto *type_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRType> *>(type);
        return std::dynamic_pointer_cast<zir::ZIRBooleanType>(*type_ptr) != nullptr;
    }

    bool zir_is_string_type(zir_type_handle type)
    {
        if (!type)
        {
            return false;
        }
        auto *type_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRType> *>(type);
        return std::dynamic_pointer_cast<zir::ZIRStringType>(*type_ptr) != nullptr;
    }

    // Value access functions
    int64_t zir_get_integer_value(zir_value_handle value)
    {
        if (!value)
        {
            return 0;
        }
        auto *value_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRValue> *>(value);
        auto int_literal = std::dynamic_pointer_cast<zir::ZIRIntegerLiteral>(*value_ptr);
        if (!int_literal)
        {
            return 0;
        }
        return int_literal->getValue();
    }

    double zir_get_float_value(zir_value_handle value)
    {
        if (!value)
        {
            return 0.0;
        }
        auto *value_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRValue> *>(value);
        auto float_literal = std::dynamic_pointer_cast<zir::ZIRFloatLiteral>(*value_ptr);
        if (!float_literal)
        {
            return 0.0;
        }
        return float_literal->getValue();
    }

    bool zir_get_bool_value(zir_value_handle value)
    {
        if (!value)
        {
            return false;
        }
        auto *value_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRValue> *>(value);
        auto bool_literal = std::dynamic_pointer_cast<zir::ZIRBooleanLiteral>(*value_ptr);
        if (!bool_literal)
        {
            return false;
        }
        return bool_literal->getValue();
    }

    const char *zir_get_string_value(zir_value_handle value)
    {
        if (!value)
        {
            return nullptr;
        }
        auto *value_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRValue> *>(value);
        auto string_literal = std::dynamic_pointer_cast<zir::ZIRStringLiteral>(*value_ptr);
        if (!string_literal)
        {
            return nullptr;
        }
        return string_literal->getValue().c_str();
    }

    // Cleanup functions
    void zir_destroy_type(zir_type_handle type)
    {
        if (!type)
        {
            return;
        }
        auto *type_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRType> *>(type);
        delete type_ptr;
    }

    void zir_destroy_value(zir_value_handle handle)
    {
        if (handle)
        {
            auto *value_ptr = reinterpret_cast<std::shared_ptr<zir::ZIRValue> *>(handle);
            delete value_ptr;
        }
    }

    // Basic block functions
    zir_block_handle zir_create_basic_block(const char *name)
    {
        if (!name)
            return nullptr;
        auto *block_ptr = new std::shared_ptr<ZIRBasicBlockImpl>(
            std::make_shared<ZIRBasicBlockImpl>(std::string(name)));
        return reinterpret_cast<zir_block_handle>(block_ptr);
    }

    void zir_destroy_basic_block(zir_block_handle handle)
    {
        auto *block_ptr = handle_to_block(handle);
        if (block_ptr)
        {
            delete block_ptr;
        }
    }

    const char *zir_get_block_name(zir_block_handle handle)
    {
        if (!handle)
            return nullptr;
        auto *block_ptr = handle_to_block(handle);
        static thread_local std::string current_name;
        current_name = (*block_ptr)->getName();
        return current_name.c_str();
    }

    void zir_set_block_name(zir_block_handle handle, const char *name)
    {
        if (!handle || !name)
            return;
        auto *block_ptr = handle_to_block(handle);
        (*block_ptr)->setName(name);
    }

    uint64_t zir_get_block_id(zir_block_handle handle)
    {
        if (!handle)
            return UINT64_MAX; // Return max value to indicate invalid block
        auto *block_ptr = handle_to_block(handle);
        return (*block_ptr)->getId();
    }

    zir_function_handle zir_create_function(const char *name)
    {
        if (!name)
            return nullptr;
        auto *function_ptr = new std::shared_ptr<ZIRFunctionImpl>(
            std::make_shared<ZIRFunctionImpl>(std::string(name)));
        return reinterpret_cast<zir_function_handle>(function_ptr);
    }

    void zir_destroy_function(zir_function_handle handle)
    {
        auto *function_ptr = handle_to_function(handle);
        if (function_ptr)
        {
            delete function_ptr;
        }
    }

    const char *zir_get_function_name(zir_function_handle handle)
    {
        if (!handle)
            return nullptr;
        auto *function_ptr = handle_to_function(handle);
        if (!function_ptr || !*function_ptr)
            return nullptr;
        return (*function_ptr)->getName().c_str();
    }

    void zir_set_function_name(zir_function_handle handle, const char *name)
    {
        if (!handle || !name)
            return;
        auto *function_ptr = handle_to_function(handle);
        if (!function_ptr || !*function_ptr || !name)
            return;
        (*function_ptr)->setName(name);
    }

    bool zir_function_add_block(zir_function_handle func_handle, zir_block_handle block_handle)
    {
        auto *function_ptr = handle_to_function(func_handle);
        auto *block_ptr = handle_to_block(block_handle);

        if (!function_ptr || !block_ptr || !*function_ptr || !*block_ptr)
        {
            return false;
        }

        (*function_ptr)->addBlock(*block_ptr);
        return true;
    }

    void zir_function_remove_block(zir_function_handle function, zir_block_handle block)
    {
        if (!function || !block)
        {
            return;
        }
        auto *function_ptr = handle_to_function(function);
        auto *block_ptr = handle_to_block(block);
        (*function_ptr)->removeBlock(*block_ptr);
        (*block_ptr)->setParentFunction(nullptr);
    }

    size_t zir_function_get_block_count(zir_function_handle handle)
    {
        if (!handle)
        {
            return 0;
        }
        auto *function_ptr = handle_to_function(handle);
        return (*function_ptr)->getBlockCount();
    }

    zir_block_handle zir_function_get_block(zir_function_handle handle, size_t index)
    {
        if (!handle)
            return nullptr;
        auto *function_ptr = handle_to_function(handle);
        auto block = (*function_ptr)->getBlock(index);
        if (!block)
            return nullptr;
        return reinterpret_cast<zir_block_handle>(block.get());
    }

    zir_instruction_handle zir_create_instruction(const char *name)
    {
        if (!name)
        {
            return nullptr;
        }
        auto instruction = std::make_shared<zir::BasicInstruction>(name);
        auto *instruction_ptr = new std::shared_ptr<zir::ZIRInstructionImpl>(std::static_pointer_cast<zir::ZIRInstructionImpl>(instruction));
        return reinterpret_cast<zir_instruction_handle>(instruction_ptr);
    }

    void zir_destroy_instruction(zir_instruction_handle handle)
    {
        if (!handle)
            return;

        auto *inst_ptr = reinterpret_cast<std::shared_ptr<ZIRInstructionImpl> *>(handle);
        delete inst_ptr;
    }

    const char *zir_get_instruction_name(zir_instruction_handle handle)
    {
        if (!handle)
            return nullptr;

        auto *inst_ptr = reinterpret_cast<std::shared_ptr<ZIRInstructionImpl> *>(handle);
        return (*inst_ptr)->getName().c_str();
    }

    zir_value_handle zir_instruction_get_left_operand(zir_instruction_handle handle)
    {
        if (!handle)
            return nullptr;

        auto *inst_ptr = reinterpret_cast<std::shared_ptr<ZIRInstructionImpl> *>(handle);
        if (auto binary_arith = std::dynamic_pointer_cast<BinaryArithmeticInst>(*inst_ptr))
        {
            auto left = binary_arith->getLeft();
            return reinterpret_cast<zir_value_handle>(new std::shared_ptr<ZIRValue>(left));
        }
        else if (auto binary_comp = std::dynamic_pointer_cast<BinaryComparisonInst>(*inst_ptr))
        {
            auto left = binary_comp->getLeft();
            return reinterpret_cast<zir_value_handle>(new std::shared_ptr<ZIRValue>(left));
        }
        else if (auto binary_logic = std::dynamic_pointer_cast<BinaryLogicalInst>(*inst_ptr))
        {
            auto left = binary_logic->getLeft();
            return reinterpret_cast<zir_value_handle>(new std::shared_ptr<ZIRValue>(left));
        }

        return nullptr;
    }

    zir_value_handle zir_instruction_get_right_operand(zir_instruction_handle handle)
    {
        if (!handle)
            return nullptr;

        auto *inst_ptr = reinterpret_cast<std::shared_ptr<ZIRInstructionImpl> *>(handle);
        if (auto binary_arith = std::dynamic_pointer_cast<BinaryArithmeticInst>(*inst_ptr))
        {
            auto right = binary_arith->getRight();
            return reinterpret_cast<zir_value_handle>(new std::shared_ptr<ZIRValue>(right));
        }
        else if (auto binary_comp = std::dynamic_pointer_cast<BinaryComparisonInst>(*inst_ptr))
        {
            auto right = binary_comp->getRight();
            return reinterpret_cast<zir_value_handle>(new std::shared_ptr<ZIRValue>(right));
        }
        else if (auto binary_logic = std::dynamic_pointer_cast<BinaryLogicalInst>(*inst_ptr))
        {
            auto right = binary_logic->getRight();
            return reinterpret_cast<zir_value_handle>(new std::shared_ptr<ZIRValue>(right));
        }

        return nullptr;
    }

    zir_value_handle zir_instruction_get_operand(zir_instruction_handle handle)
    {
        if (!handle)
            return nullptr;

        auto *inst_ptr = reinterpret_cast<std::shared_ptr<ZIRInstructionImpl> *>(handle);
        if (auto unary = std::dynamic_pointer_cast<UnaryLogicalInst>(*inst_ptr))
        {
            auto operand = unary->getOperand();
            return reinterpret_cast<zir_value_handle>(new std::shared_ptr<ZIRValue>(operand));
        }

        return nullptr;
    }

    void zir_block_add_instruction(zir_block_handle block, zir_instruction_handle instruction)
    {
        if (!block || !instruction)
        {
            return;
        }
        auto *block_ptr = handle_to_block(block);
        auto *instruction_ptr = reinterpret_cast<std::shared_ptr<ZIRInstructionImpl> *>(instruction);
        (*block_ptr)->addInstruction(*instruction_ptr);
    }

    void zir_block_remove_instruction(zir_block_handle block, size_t index)
    {
        if (!block)
        {
            return;
        }
        auto *block_ptr = handle_to_block(block);
        (*block_ptr)->removeInstruction(index);
    }

    zir_instruction_handle zir_block_get_instruction(zir_block_handle block, size_t index)
    {
        if (!block)
        {
            return nullptr;
        }
        auto *block_ptr = handle_to_block(block);
        auto instruction = (*block_ptr)->getInstruction(index);
        if (!instruction)
        {
            return nullptr;
        }
        auto *handle = new std::shared_ptr<ZIRInstructionImpl>(instruction);
        return reinterpret_cast<zir_instruction_handle>(handle);
    }

    size_t zir_block_get_instruction_count(zir_block_handle block)
    {
        if (!block)
        {
            return 0;
        }
        auto *block_ptr = handle_to_block(block);
        return (*block_ptr)->getInstructionCount();
    }

    zir_instruction_handle zir_create_add_instruction(zir_value_handle left, zir_value_handle right)
    {
        if (!left || !right)
            return nullptr;
        auto left_val = handle_to_value(left);
        auto right_val = handle_to_value(right);
        if (!left_val || !right_val)
            return nullptr;

        auto left_value = std::static_pointer_cast<ZIRValue>(*left_val);
        auto right_value = std::static_pointer_cast<ZIRValue>(*right_val);

        auto inst = std::make_shared<AddInst>(left_value, right_value);
        return reinterpret_cast<zir_instruction_handle>(new std::shared_ptr<ZIRInstructionImpl>(inst));
    }

    zir_instruction_handle zir_create_sub_instruction(zir_value_handle left, zir_value_handle right)
    {
        if (!left || !right)
            return nullptr;
        auto left_val = handle_to_value(left);
        auto right_val = handle_to_value(right);
        if (!left_val || !right_val)
            return nullptr;

        auto left_value = std::static_pointer_cast<ZIRValue>(*left_val);
        auto right_value = std::static_pointer_cast<ZIRValue>(*right_val);

        auto inst = std::make_shared<SubInst>(left_value, right_value);
        return reinterpret_cast<zir_instruction_handle>(new std::shared_ptr<ZIRInstructionImpl>(inst));
    }

    zir_instruction_handle zir_create_mul_instruction(zir_value_handle left, zir_value_handle right)
    {
        if (!left || !right)
            return nullptr;
        auto left_val = handle_to_value(left);
        auto right_val = handle_to_value(right);
        if (!left_val || !right_val)
            return nullptr;

        auto left_value = std::static_pointer_cast<ZIRValue>(*left_val);
        auto right_value = std::static_pointer_cast<ZIRValue>(*right_val);

        auto inst = std::make_shared<MulInst>(left_value, right_value);
        return reinterpret_cast<zir_instruction_handle>(new std::shared_ptr<ZIRInstructionImpl>(inst));
    }

    zir_instruction_handle zir_create_div_instruction(zir_value_handle left, zir_value_handle right)
    {
        if (!left || !right)
            return nullptr;
        auto left_val = handle_to_value(left);
        auto right_val = handle_to_value(right);
        if (!left_val || !right_val)
            return nullptr;

        auto left_value = std::static_pointer_cast<ZIRValue>(*left_val);
        auto right_value = std::static_pointer_cast<ZIRValue>(*right_val);

        auto inst = std::make_shared<DivInst>(left_value, right_value);
        return reinterpret_cast<zir_instruction_handle>(new std::shared_ptr<ZIRInstructionImpl>(inst));
    }

    zir_instruction_handle zir_create_mod_instruction(zir_value_handle left, zir_value_handle right)
    {
        if (!left || !right)
            return nullptr;
        auto left_val = handle_to_value(left);
        auto right_val = handle_to_value(right);
        if (!left_val || !right_val)
            return nullptr;

        auto left_value = std::static_pointer_cast<ZIRValue>(*left_val);
        auto right_value = std::static_pointer_cast<ZIRValue>(*right_val);

        auto inst = std::make_shared<ModInst>(left_value, right_value);
        return reinterpret_cast<zir_instruction_handle>(new std::shared_ptr<ZIRInstructionImpl>(inst));
    }

    zir_instruction_handle zir_create_pow_instruction(zir_value_handle left, zir_value_handle right)
    {
        if (!left || !right)
            return nullptr;
        auto left_val = handle_to_value(left);
        auto right_val = handle_to_value(right);
        if (!left_val || !right_val)
            return nullptr;

        auto left_value = std::static_pointer_cast<ZIRValue>(*left_val);
        auto right_value = std::static_pointer_cast<ZIRValue>(*right_val);

        auto inst = std::make_shared<PowInst>(left_value, right_value);
        return reinterpret_cast<zir_instruction_handle>(new std::shared_ptr<ZIRInstructionImpl>(inst));
    }

    zir_instruction_handle zir_create_eq_instruction(zir_value_handle left, zir_value_handle right)
    {
        if (!left || !right)
            return nullptr;
        auto left_val = handle_to_value(left);
        auto right_val = handle_to_value(right);
        if (!left_val || !right_val)
            return nullptr;

        auto left_value = std::static_pointer_cast<ZIRValue>(*left_val);
        auto right_value = std::static_pointer_cast<ZIRValue>(*right_val);

        auto inst = std::make_shared<EqInst>(left_value, right_value);
        return reinterpret_cast<zir_instruction_handle>(new std::shared_ptr<ZIRInstructionImpl>(inst));
    }

    zir_instruction_handle zir_create_ne_instruction(zir_value_handle left, zir_value_handle right)
    {
        if (!left || !right)
            return nullptr;
        auto left_val = handle_to_value(left);
        auto right_val = handle_to_value(right);
        if (!left_val || !right_val)
            return nullptr;

        auto left_value = std::static_pointer_cast<ZIRValue>(*left_val);
        auto right_value = std::static_pointer_cast<ZIRValue>(*right_val);

        auto inst = std::make_shared<NeInst>(left_value, right_value);
        return reinterpret_cast<zir_instruction_handle>(new std::shared_ptr<ZIRInstructionImpl>(inst));
    }

    zir_instruction_handle zir_create_lt_instruction(zir_value_handle left, zir_value_handle right)
    {
        if (!left || !right)
            return nullptr;
        auto left_val = handle_to_value(left);
        auto right_val = handle_to_value(right);
        if (!left_val || !right_val)
            return nullptr;

        auto left_value = std::static_pointer_cast<ZIRValue>(*left_val);
        auto right_value = std::static_pointer_cast<ZIRValue>(*right_val);

        auto inst = std::make_shared<LtInst>(left_value, right_value);
        return reinterpret_cast<zir_instruction_handle>(new std::shared_ptr<ZIRInstructionImpl>(inst));
    }

    zir_instruction_handle zir_create_le_instruction(zir_value_handle left, zir_value_handle right)
    {
        if (!left || !right)
            return nullptr;
        auto left_val = handle_to_value(left);
        auto right_val = handle_to_value(right);
        if (!left_val || !right_val)
            return nullptr;

        auto left_value = std::static_pointer_cast<ZIRValue>(*left_val);
        auto right_value = std::static_pointer_cast<ZIRValue>(*right_val);

        auto inst = std::make_shared<LeInst>(left_value, right_value);
        return reinterpret_cast<zir_instruction_handle>(new std::shared_ptr<ZIRInstructionImpl>(inst));
    }

    zir_instruction_handle zir_create_gt_instruction(zir_value_handle left, zir_value_handle right)
    {
        if (!left || !right)
            return nullptr;
        auto left_val = handle_to_value(left);
        auto right_val = handle_to_value(right);
        if (!left_val || !right_val)
            return nullptr;

        auto left_value = std::static_pointer_cast<ZIRValue>(*left_val);
        auto right_value = std::static_pointer_cast<ZIRValue>(*right_val);

        auto inst = std::make_shared<GtInst>(left_value, right_value);
        return reinterpret_cast<zir_instruction_handle>(new std::shared_ptr<ZIRInstructionImpl>(inst));
    }

    zir_instruction_handle zir_create_ge_instruction(zir_value_handle left, zir_value_handle right)
    {
        if (!left || !right)
            return nullptr;
        auto left_val = handle_to_value(left);
        auto right_val = handle_to_value(right);
        if (!left_val || !right_val)
            return nullptr;

        auto left_value = std::static_pointer_cast<ZIRValue>(*left_val);
        auto right_value = std::static_pointer_cast<ZIRValue>(*right_val);

        auto inst = std::make_shared<GeInst>(left_value, right_value);
        return reinterpret_cast<zir_instruction_handle>(new std::shared_ptr<ZIRInstructionImpl>(inst));
    }

    zir_instruction_handle zir_create_and_instruction(zir_value_handle left, zir_value_handle right)
    {
        if (!left || !right)
            return nullptr;
        auto left_val = handle_to_value(left);
        auto right_val = handle_to_value(right);
        if (!left_val || !right_val)
            return nullptr;

        auto left_value = std::static_pointer_cast<ZIRValue>(*left_val);
        auto right_value = std::static_pointer_cast<ZIRValue>(*right_val);

        auto inst = std::make_shared<AndInst>(left_value, right_value);
        return reinterpret_cast<zir_instruction_handle>(new std::shared_ptr<ZIRInstructionImpl>(inst));
    }

    zir_instruction_handle zir_create_or_instruction(zir_value_handle left, zir_value_handle right)
    {
        if (!left || !right)
            return nullptr;
        auto left_val = handle_to_value(left);
        auto right_val = handle_to_value(right);
        if (!left_val || !right_val)
            return nullptr;

        auto left_value = std::static_pointer_cast<ZIRValue>(*left_val);
        auto right_value = std::static_pointer_cast<ZIRValue>(*right_val);

        auto inst = std::make_shared<OrInst>(left_value, right_value);
        return reinterpret_cast<zir_instruction_handle>(new std::shared_ptr<ZIRInstructionImpl>(inst));
    }

    zir_instruction_handle zir_create_not_instruction(zir_value_handle operand)
    {
        if (!operand)
            return nullptr;
        auto operand_val = handle_to_value(operand);
        if (!operand_val)
            return nullptr;

        auto operand_value = std::static_pointer_cast<ZIRValue>(*operand_val);

        auto inst = std::make_shared<NotInst>(operand_value);
        return reinterpret_cast<zir_instruction_handle>(new std::shared_ptr<ZIRInstructionImpl>(inst));
    }

    zir_instruction_handle zir_create_void_return_instruction(void)
    {
        auto ret = std::make_shared<zir::ReturnInst>();
        return reinterpret_cast<zir_instruction_handle>(new std::shared_ptr<zir::ZIRInstructionImpl>(ret));
    }

    zir_value_handle zir_branch_get_condition(zir_instruction_handle branch)
    {
        if (!branch)
        {
            return nullptr;
        }
        auto inst = *handle_to_instruction(branch);
        if (!inst)
        {
            return nullptr;
        }
        auto branch_inst = std::dynamic_pointer_cast<zir::BranchInst>(inst);
        if (!branch_inst)
        {
            return nullptr;
        }
        return reinterpret_cast<zir_value_handle>(new std::shared_ptr<zir::ZIRValue>(branch_inst->getCondition()));
    }

    zir_block_handle zir_branch_get_true_block(zir_instruction_handle branch)
    {
        if (!branch)
            return nullptr;
        auto inst_ptr = handle_to_instruction(branch);
        if (!inst_ptr || !*inst_ptr)
            return nullptr;
        auto branch_inst = dynamic_cast<BranchInst *>(inst_ptr->get());
        if (!branch_inst)
            return nullptr;
        return branch_inst->getTrueBlockHandle();
    }

    zir_block_handle zir_branch_get_false_block(zir_instruction_handle branch)
    {
        if (!branch)
            return nullptr;
        auto inst_ptr = handle_to_instruction(branch);
        if (!inst_ptr || !*inst_ptr)
            return nullptr;
        auto branch_inst = dynamic_cast<BranchInst *>(inst_ptr->get());
        if (!branch_inst)
            return nullptr;
        return branch_inst->getFalseBlockHandle();
    }

    zir_block_handle zir_jump_get_target(zir_instruction_handle jump)
    {
        if (!jump)
            return nullptr;
        auto inst_ptr = handle_to_instruction(jump);
        if (!inst_ptr || !*inst_ptr)
            return nullptr;
        auto jump_inst = dynamic_cast<JumpInst *>(inst_ptr->get());
        if (!jump_inst)
            return nullptr;
        return jump_inst->getTargetHandle();
    }

    zir_value_handle zir_return_get_value(zir_instruction_handle ret)
    {
        if (!ret)
            return nullptr;

        auto *inst_ptr = reinterpret_cast<std::shared_ptr<ZIRInstructionImpl> *>(ret);
        auto return_inst = std::dynamic_pointer_cast<ReturnInst>(*inst_ptr);
        if (!return_inst || !return_inst->getValue())
            return nullptr;

        auto *value_ptr = new std::shared_ptr<ZIRValue>(return_inst->getValue());
        return reinterpret_cast<zir_value_handle>(value_ptr);
    }

    bool zir_return_is_void(zir_instruction_handle ret)
    {
        if (!ret)
            return false;

        auto *inst_ptr = reinterpret_cast<std::shared_ptr<ZIRInstructionImpl> *>(ret);
        auto return_inst = std::dynamic_pointer_cast<ReturnInst>(*inst_ptr);
        if (!return_inst)
            return false;

        return return_inst->getValue() == nullptr;
    }

    void zir_set_block_parent(zir_block_handle block, zir_function_handle function)
    {
        std::cout << "C-API Debug: Setting block parent, function handle=" << function << std::endl;
        if (!block)
            return;
        auto *block_ptr = handle_to_block(block);
        (*block_ptr)->setParentFunction(function);
        std::cout << "C-API Debug: Block parent set to " << (*block_ptr)->getParentFunction() << std::endl;
    }

    zir_function_handle zir_get_block_parent(zir_block_handle block)
    {
        if (!block)
            return nullptr;
        auto *block_ptr = handle_to_block(block);
        return (*block_ptr)->getParentFunction();
    }

    size_t zir_function_remove_dead_blocks(zir_function_handle handle)
    {
        auto *function_ptr = handle_to_function(handle);
        if (!function_ptr || !*function_ptr)
        {
            return 0;
        }
        return (*function_ptr)->removeDeadBlocks();
    }

    bool zir_block_is_dead(zir_block_handle block, zir_function_handle func_handle)
    {
        if (!block || !func_handle)
        {
            return false;
        }

        auto *function_ptr = handle_to_function(func_handle);
        auto *block_ptr = handle_to_block(block);

        if (!function_ptr || !block_ptr || !*function_ptr || !*block_ptr)
        {
            return false;
        }

        // First check if the block belongs to this function
        bool found = false;
        for (size_t i = 0; i < (*function_ptr)->getBlockCount(); i++)
        {
            if ((*function_ptr)->getBlock(i) == *block_ptr)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            return false;
        }

        // Now check if it's in the dead blocks list
        auto dead_blocks = (*function_ptr)->findDeadBlocks();
        return std::find(dead_blocks.begin(), dead_blocks.end(), *block_ptr) != dead_blocks.end();
    }

    // Value management
    zir_value_handle zir_create_int32_value(int32_t value)
    {
        auto *value_ptr = new std::shared_ptr<ZIRValueImpl>(
            std::make_shared<ZIRInt32Value>(value));
        return reinterpret_cast<zir_value_handle>(value_ptr);
    }

    zir_value_handle zir_create_int64_value(int64_t value)
    {
        auto *value_ptr = new std::shared_ptr<ZIRValueImpl>(
            std::make_shared<ZIRInt64Value>(value));
        return reinterpret_cast<zir_value_handle>(value_ptr);
    }

    zir_value_handle zir_create_float_value(float value)
    {
        auto *value_ptr = new std::shared_ptr<ZIRValueImpl>(
            std::make_shared<ZIRFloatValue>(value));
        return reinterpret_cast<zir_value_handle>(value_ptr);
    }

    zir_value_handle zir_create_double_value(double value)
    {
        auto *value_ptr = new std::shared_ptr<ZIRValueImpl>(
            std::make_shared<ZIRDoubleValue>(value));
        return reinterpret_cast<zir_value_handle>(value_ptr);
    }

    zir_value_handle zir_create_bool_value(bool value)
    {
        auto *value_ptr = new std::shared_ptr<ZIRValueImpl>(
            std::make_shared<ZIRBoolValue>(value));
        return reinterpret_cast<zir_value_handle>(value_ptr);
    }

    zir_instruction_handle zir_create_return_instruction(zir_value_handle value)
    {
        if (!value)
        {
            return nullptr;
        }

        auto value_ptr = handle_to_value(value);
        if (!value_ptr || !(*value_ptr))
        {
            return nullptr;
        }

        auto inst = std::make_shared<zir::ReturnInst>(*value_ptr);
        return reinterpret_cast<zir_instruction_handle>(new std::shared_ptr<zir::ZIRInstructionImpl>(inst));
    }

    zir_instruction_handle zir_create_branch_instruction(zir_value_handle condition,
                                                         zir_block_handle true_target,
                                                         zir_block_handle false_target)
    {
        if (!condition || !true_target || !false_target)
            return nullptr;

        auto cond_ptr = handle_to_value(condition);
        auto true_ptr = handle_to_block(true_target);
        auto false_ptr = handle_to_block(false_target);

        if (!cond_ptr || !*cond_ptr || !true_ptr || !*true_ptr || !false_ptr || !*false_ptr)
            return nullptr;

        // Store the original block handles in the branch instruction
        auto branch = std::make_shared<BranchInst>(*cond_ptr, *true_ptr, *false_ptr);
        branch->setTrueBlockHandle(true_target);
        branch->setFalseBlockHandle(false_target);
        return reinterpret_cast<zir_instruction_handle>(new std::shared_ptr<ZIRInstructionImpl>(branch));
    }

    zir_instruction_handle zir_create_jump_instruction(zir_block_handle target)
    {
        if (!target)
        {
            return nullptr;
        }

        auto target_ptr = handle_to_block(target);
        if (!target_ptr || !(*target_ptr))
        {
            return nullptr;
        }

        // Store the original block handle in the jump instruction
        auto inst = std::make_shared<JumpInst>(*target_ptr);
        inst->setTargetHandle(target);
        return reinterpret_cast<zir_instruction_handle>(new std::shared_ptr<ZIRInstructionImpl>(inst));
    }
}