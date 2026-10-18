#include "evaluator.h"
#include <list>
#include <string>
#include <string_view>
#include "utils.h"

#include <iostream>

namespace ekt
{
namespace {
    enum class Type
    {
        Arg,
        Op,
        Literal,
        Evaluated
    };

    enum class OpType
    {
        None,
        Equal,
        NotEqual,
        Or,
        And
    };

    struct Node
    {
        Type type;
        OpType op_type;
        std::string value;
        bool evaluated_value = false;
    };

    void parse_predicate(const Scope& scope, const std::string_view& pred_view, std::list<Node>& nodes)
    {
        if(pred_view.empty())
        {
            return;
        }

        size_t pos = 0;
        size_t current_var_start = 0;
        while(pos < pred_view.size())
        {
            OpType type = OpType::None;
            const auto op_str = pred_view.substr(pos, 2);
            int arg_size = 0;
            if (op_str == "==")
            {
                type = OpType::Equal;
                arg_size = 2;
            }
            else if (op_str == "!=")
            {
                type = OpType::NotEqual;
                arg_size = 2;
            }
            else if (op_str == "||")
            {
                type = OpType::Or;
                arg_size = 2;
            }
            else if (op_str == "&&")
            {
                type = OpType::And;
                arg_size = 2;
            }

            if (type != OpType::None)
            {
                if (pos - current_var_start > 0)
                {
                    std::string value;
                    auto var = utils::trim(pred_view.substr(current_var_start, pos - current_var_start));
                    if(var.front() == '"' && var.back() == '"')
                    {
                        value = value = var.substr(1, var.size() - 2);
                    }
                    else {
                        // EKTODO string_view versions here?
                        value = scope.get_value(std::string(var));
                    }
                    nodes.push_back(Node{
                        .type = Type::Arg,
                        .value = value
                    });
                }

                nodes.push_back(Node{
                    .type = Type::Op,
                    .op_type = type
                });
                pos += arg_size;
                current_var_start = pos;
            }
            else {
                pos++;
            }
        }

        if (pos - current_var_start <= 0)
        {
            return;
        }
        std::string value;
        auto var = utils::trim(pred_view.substr(current_var_start, pos - current_var_start));
        if(var.front() == '"' && var.back() == '"')
        {
            value = var.substr(1, var.size() - 2);
        }
        else {
            // EKTODO string_view versions here?
            value = scope.get_value(std::string(var));
        }
        nodes.push_back(Node{
            .type = Type::Arg,
            .value = value
        });
    }
}

bool evaluate_condition(const Scope& scope, const std::string& predicate)
{
    std::list<Node> nodes;
    parse_predicate(scope, predicate, nodes);

    auto current = nodes.begin();
    int size = nodes.size();
    while(!nodes.empty())
    {
        if(nodes.size() == 1 && current->type == Type::Evaluated)
        {
            return current->evaluated_value;
        }
        if(current->type == Type::Op)
        {
            if (current == nodes.begin() || current == nodes.end() || std::next(current) == nodes.end())
            {
                // EKTODO error
                return false;
            }

            auto left = std::prev(current);
            auto right = std::next(current);
            switch (current->op_type) {
                case OpType::None:
                    continue;
                case OpType::Equal:
                case OpType::NotEqual:
                {
                    if(left->type != Type::Arg || right->type != Type::Arg)
                    {
                        // EKTODO error
                        return false;
                    }

                    bool eval = current->op_type == OpType::Equal ? left->value == right->value : left->value != right->value;
                    left->type = Type::Evaluated;
                    left->evaluated_value = eval;
                    nodes.erase(current);
                    nodes.erase(right);
                    current = std::next(left);
                    break;
                }
                case OpType::Or:
                case OpType::And:
                {
                    auto left = std::prev(current);
                    auto right = std::next(current);
                    if (right->type != Type::Evaluated)
                    {
                        break;
                    }
                    bool eval = current->op_type == OpType::Or ? left->evaluated_value || right->evaluated_value :
                        left->evaluated_value && right->evaluated_value;
                    left->type = Type::Evaluated;
                    left->evaluated_value = eval;
                    nodes.erase(current);
                    nodes.erase(right);
                    current = std::next(left);
                    break;
                }
            }
        }

        current++;
        if (current == nodes.end())
        {
            // Nothing was removed, infinite loop
            if (size == nodes.size())
            {
                return false;
            }
            size = nodes.size();
            current = nodes.begin();
        }
    }

    return false;
}

}
