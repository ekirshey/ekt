#include "EvalNode.h"

namespace ekt
{
    bool is_type(Type in, Type compare)
    {
        return (static_cast<int>(in) & static_cast<int>(compare)) != 0;
    }

    bool is_primitive(Type type)
    {
        return is_type(type, Type::Literal) || is_type(type, Type::Bool) || is_type(type, Type::Number);
    }

    EvalNode::EvalNode(const Op& value)
        : op(value)
        , type(Type::Op)
    {
    }

    EvalNode::EvalNode(const std::string& value)
        : literal_value(value)
        , type(Type::Literal)
    {
    }

    EvalNode::EvalNode(bool value)
        : bool_value(value)
        , type(Type::Bool)
    {
    }

    EvalNode::EvalNode(int value)
        : number_value(value)
        , type(Type::Number)
    {
    }

    EvalNode::EvalNode(const FuncReturnVal& val)
    {
        switch(val.type)
        {
            case FuncReturnType::Bool:
                type = Type::Bool;
                bool_value = val.bool_value;
                break;
            case FuncReturnType::Number:
                type = Type::Number;
                number_value = val.number_value;
                break;
        }
    }

    bool EvalNode::operator==(const EvalNode& rhs) const
    {
        if(rhs.type != type)
        {
            return false;
        }

        if(is_type(type, Type::Bool))
        {
            return bool_value == rhs.bool_value;
        }
        else if(is_type(type, Type::Literal))
        {
            return literal_value == rhs.literal_value;
        }
        else if(is_type(type, Type::Number))
        {
            return number_value == rhs.number_value;
        }

        return false;
    }

    bool EvalNode::operator!=(const EvalNode& rhs) const
    {
        return !(*this == rhs);
    }

    bool EvalNode::operator<(const EvalNode& rhs) const
    {
        if(rhs.type != type)
        {
            return false;
        }

        // Only numbers for now
        if(!is_type(type, Type::Number))
        {
            return false;
        }

        return number_value < rhs.number_value;
    }

    bool EvalNode::operator<=(const EvalNode& rhs) const
    {
        if(rhs.type != type)
        {
            return false;
        }

        // Only numbers for now
        if(!is_type(type, Type::Number))
        {
            return false;
        }

        return !(rhs < *this);
    }

    bool EvalNode::operator>(const EvalNode& rhs) const
    {
        if(rhs.type != type)
        {
            return false;
        }

        // Only numbers for now
        if(!is_type(type, Type::Number))
        {
            return false;
        }

        return rhs < *this;
    }

    bool EvalNode::operator>=(const EvalNode& rhs) const
    {
        if(rhs.type != type)
        {
            return false;
        }

        // Only numbers for now
        if(!is_type(type, Type::Number))
        {
            return false;
        }

        return !(*this < rhs);
    }

    bool EvalNode::operator||(const EvalNode& rhs) const
    {
        if(rhs.type != type)
        {
            return false;
        }

        if(!is_type(type, Type::Bool))
        {
            return false;
        }

        return bool_value || rhs.bool_value;
    }

    bool EvalNode::operator&&(const EvalNode& rhs) const
    {
        if(rhs.type != type)
        {
            return false;
        }

        if(!is_type(type, Type::Bool))
        {
            return false;
        }

        return bool_value && rhs.bool_value;
    }
}