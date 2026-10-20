#include "evaluator.h"
#include <functional>
#include <list>
#include <string>
#include <string_view>
#include "utils.h"

#include <iostream>
#include <unordered_map>

namespace ekt
{
namespace {
    enum class Type
    {
        None,
        Arg,
        Op,
        Literal,
        Evaluated
    };

    enum class NewType
    {
        Bool,
        Literal,
        Op
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

    struct Op
    {
        OpType type = OpType::None;
        size_t length = 0;
    };

    std::unordered_map<std::string, Op> ops = {
      {"==", {.type = OpType::Equal, .length = 2}},
      {"!=", {.type = OpType::NotEqual, .length = 2}},
      {"||", {.type = OpType::Or, .length = 2}},
      {"&&", {.type = OpType::And, .length = 2}},
    };

    std::optional<Node> create_arg(std::string_view pred_view, size_t start, size_t pos, const Scope& scope)
    {
        std::string value;
        auto var = utils::trim(pred_view.substr(start, pos - start));
        if(var.empty())
        {
            return std::nullopt;
        }

        if(var.front() == '"' && var.back() == '"')
        {
            value = value = var.substr(1, var.size() - 2);
        }
        else {
            // EKTODO string_view versions here?
            value = scope.get_value(std::string(var));
        }

        return Node{
            .type = Type::Arg,
            .value = value
        };
    }

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
            const auto op_str = std::string(pred_view.substr(pos, 2));
            auto op_result = ops.find(op_str);

            if (op_result != ops.end())
            {
                auto& op = op_result->second;
                if (pos - current_var_start > 0)
                {
                    auto node = create_arg(pred_view, current_var_start, pos, scope);
                    if(node != std::nullopt)
                    {
                        nodes.push_back(*node);
                    }
                }

                nodes.push_back(Node{
                    .type = Type::Op,
                    .op_type = op.type
                });
                pos += op.length;
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
        auto node = create_arg(pred_view, current_var_start, pos, scope);
        if(node != std::nullopt)
        {
            nodes.push_back(*node);
        }
    }

    bool can_perform_op(std::list<Node>::iterator& current, std::list<Node>::iterator& left, std::list<Node>::iterator& right, OpType type)
    {
        switch (type) {
            case OpType::None:
                return true;
            case OpType::Equal:
            case OpType::NotEqual:
                return (left->type == Type::Arg && right->type == Type::Arg);
            case OpType::Or:
            case OpType::And:
                return (left->type == Type::Evaluated && right->type == Type::Evaluated);
        }

        return true;
    }

    void perform_op(std::list<Node>& nodes, std::list<Node>::iterator& current, std::list<Node>::iterator& left, std::list<Node>::iterator& right, OpType type)
    {
        bool eval = false;
        switch (type) {
            case OpType::None:
                return;
            case OpType::Equal:
                eval = left->value == right->value;
                break;
            case OpType::NotEqual:
                eval = left->value != right->value;
                break;
            case OpType::Or:
                eval = left->evaluated_value || right->evaluated_value;
                break;
            case OpType::And:
                eval = left->evaluated_value && right->evaluated_value;
                break;
        }

        left->type = Type::Evaluated;
        left->evaluated_value = eval;
        nodes.erase(current);
        nodes.erase(right);
        current = std::next(left);
    }
}


/*
Proper way is a tree

bool,string_literal

                


*/

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

        if(current != nodes.end() && current->type == Type::Op)
        {
            if (current == nodes.begin() || current == nodes.end() || std::next(current) == nodes.end())
            {
                // EKTODO error
                return false;
            }

            auto left = std::prev(current);
            auto right = std::next(current);
            if(can_perform_op(current, left, right, current->op_type))
            {
                perform_op(nodes, current, left, right, current->op_type);
            }
        }

        if (current == nodes.end())
        {
            // Nothing was removed and it's an infinite loop
            if (size == nodes.size())
            {
                return false;
            }
            size = nodes.size();
            current = nodes.begin();
        }
        else
        {
            current++;
        }
    }

    return false;
}

bool is_function(const std::string& predicate)
{
    return false;
}

}
