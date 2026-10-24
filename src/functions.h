#pragma once
#include <string>
#include <functional>

#include "Scope.h"


namespace ekt
{
    enum class FuncArgType
    {
        Literal,
        Number
    };

    enum class FuncReturnType
    {
        Bool,
        Number,
        Literal
    };

    struct FuncArg
    {
        FuncArgType type;
        std::string literal_value;
        int number_value;
    };

    struct FuncReturnVal
    {
        FuncReturnType type;
        bool bool_value;
        int number_value;
        std::string literal_value;
    };

    struct FunctionDefinition
    {
        std::vector<FuncArgType> arg_types;
        std::function<FuncReturnVal(const std::vector<FuncArg>&, const Scope&)> impl;
    };

    struct Function
    {
        std::vector<FuncArg> args;
        FunctionDefinition def;

        FuncReturnVal evaluate(const Scope& scope)
        {
            return def.impl(args, scope);
        }
    };

    std::optional<Function> is_function(const std::string_view& predicate);
}
