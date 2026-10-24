#pragma once

#include "functions.h"

namespace ekt
{
    enum class Type
    {
        None = 0,
        Op = 1 << 0,
        Literal = 1 << 1,
        Bool = 1 << 2,
        Number = 1 << 3,

        Primitive = Literal | Bool | Number
    };

    bool is_type(Type in, Type compare);
    bool is_primitive(Type type);

    enum class OpType
    {
        None,
        Equal,
        NotEqual,
        Or,
        And,
        GreaterThan,
        GreaterThanOrEqual,
        LessThan,
        LessThanOrEqual,
        Not,
        OpenParen,
        CloseParen
    };

    struct Op
    {
        OpType type = OpType::None;
        size_t length = 0;
        int precedence = -1;
        int num_args = 0;
        int required_types = 0;
    };

    struct EvalNode
    {
        EvalNode() = default;
        EvalNode(const Op& value);
        EvalNode(const std::string& value);
        EvalNode(bool value);
        EvalNode(int value);
        EvalNode(const FuncReturnVal& val);

        bool operator==(const EvalNode& rhs) const;
        bool operator!=(const EvalNode& rhs) const;
        bool operator<(const EvalNode& rhs) const;
        bool operator<=(const EvalNode& rhs) const;
        bool operator>(const EvalNode& rhs) const;
        bool operator>=(const EvalNode& rhs) const;
        bool operator||(const EvalNode& rhs) const;
        bool operator&&(const EvalNode& rhs) const;
        bool operator!() const;

        Type type = Type::None;
        Op op{};
        std::string literal_value;
        bool bool_value = false;
        int number_value = 0;
        std::vector<EvalNode> children{};
    };

}