#include "evaluator.h"

#include <functional>
#include <list>
#include <string>
#include <string_view>
#include "utils.h"
#include "functions.h"
#include "EvalNode.h"

#include <iostream>
#include <unordered_map>
#include <unordered_set>

namespace ekt
{
namespace {

    std::unordered_set<char> op_chars = {'=', '!', '<', '>', '|', '&'};

    std::unordered_map<std::string, Op> ops = {
      // Equality
      {"==", {.type = OpType::Equal, .length = 2, .precedence = 3, .num_args = 2, .required_types = static_cast<int>(Type::Primitive)}},
      {"!=", {.type = OpType::NotEqual, .length = 2, .precedence = 3, .num_args = 2, .required_types = static_cast<int>(Type::Primitive)}},

      // Comparisons
      {"<", {.type = OpType::LessThan, .length = 2, .precedence = 3, .num_args = 2, .required_types = static_cast<int>(Type::Number)}},
      {">", {.type = OpType::GreaterThan, .length = 2, .precedence = 3, .num_args = 2, .required_types = static_cast<int>(Type::Number)}},
      {"<=", {.type = OpType::LessThanOrEqual, .length = 2, .precedence = 3, .num_args = 2, .required_types = static_cast<int>(Type::Number)}},
      {">=", {.type = OpType::GreaterThanOrEqual, .length = 2, .precedence = 3, .num_args = 2, .required_types = static_cast<int>(Type::Number)}},

      // Logical Ops
      {"||", {.type = OpType::Or, .length = 2, .precedence = 2, .num_args = 2, .required_types = static_cast<int>(Type::Bool)}},
      {"&&", {.type = OpType::And, .length = 2, .precedence = 2, .num_args = 2, .required_types = static_cast<int>(Type::Bool)}}
    };

    std::optional<EvalNode> create_token(std::string_view pred_view, size_t start, size_t pos, const Scope& scope)
    {
        std::string value;
        auto var = utils::trim(pred_view.substr(start, pos - start));
        if(var.empty())
        {
            return std::nullopt;
        }
        
        auto func = is_function(var);
        if(func != std::nullopt)
        {
            return EvalNode(func->evaluate(scope));
        }

        auto num = utils::to_int(var);
        if (num)
        {
            return EvalNode(num.value());
        }

        if(var.front() == '"' && var.back() == '"')
        {
            value = value = var.substr(1, var.size() - 2);
        }
        else {
            // EKTODO string_view versions here?
            value = scope.get_value(std::string(var));
        }

        // Convert string values to numbers if appropriate
        num = utils::to_int(value);
        if (num)
        {
            return EvalNode(num.value());
        }

        return EvalNode(value);
    }

    void tokenize(const std::string_view& pred_view, const Scope& scope, std::vector<EvalNode>& tokens)
    {
        size_t pos = 0;
        size_t current_var_start = 0;
        while(pos < pred_view.size())
        {
            int end_op_pos = pos;

            while(end_op_pos < pred_view.size() && op_chars.contains(pred_view[end_op_pos]))
            {
                end_op_pos++;
            }

            const auto op_str = std::string(pred_view.substr(pos, end_op_pos - pos));
            auto op_result = ops.find(op_str);

            if (op_result != ops.end())
            {
                auto& op = op_result->second;

                if (pos - current_var_start > 0)
                {
                    auto arg_node = create_token(pred_view, current_var_start, pos, scope);
                    if(arg_node != std::nullopt)
                    {
                        tokens.push_back(*arg_node);
                    }
                }

                tokens.push_back(EvalNode(op));

                pos += op.length;
                current_var_start = pos;
            }
            else {
                pos++;
            }
        }

        if (pos - current_var_start > 0)
        {
            auto node = create_token(pred_view, current_var_start, pos, scope);
            if(node != std::nullopt)
            {
                tokens.push_back(*node);
            }
        }
    }

    EvalNode create_tree(const std::vector<EvalNode>& tokens)
    {
        if (tokens.empty())
        {
            return {};
        }

        EvalNode root = tokens[0];
        EvalNode* current = &root;
        for(int i = 0; i < tokens.size(); i++)
        {
            auto token = tokens[i];

            // Add non ops to the current node
            if (is_primitive(token.type))
            {
                current->children.push_back(token);
                continue;
            }

            if (current->type != Type::Op || token.op.precedence < current->op.precedence)
            {
                token.children.push_back(root);
                root = token;
                current = &root;
            }
            else
            {
                while(current->children.size() > 1)
                {
                    token.children.push_back(current->children.back());
                    current->children.pop_back();
                }
                current->children.push_back(token);
                current = &current->children[current->children.size() - 1];
            }
        }

        return root;
    }

    std::expected<void, std::string> resolve_tree(EvalNode& node)
    {
        if(node.type != Type::Op)
        {
            return {};
        }

        for (auto& n : node.children)
        {
            auto result = resolve_tree(n);
            if (!result.has_value())
            {
                return std::unexpected(result.error());
            }
        }

        if (node.children.size() != node.op.num_args)
        {
            return std::unexpected("Invalid argument count");
        }

        Type prev_type = Type::None;
        for(auto& c : node.children)
        {
            if ((static_cast<int>(c.type) & node.op.required_types) == 0)
            {
                return std::unexpected("Incorrect types");
            }

            if(prev_type == Type::None)
            {
                prev_type = c.type;
            }
            else if(prev_type != c.type)
            {
                return std::unexpected("Args must be of the same type");
            }
        }

        bool eval = false;
        switch(node.op.type)
        {
            case OpType::Equal:
            {
                eval = node.children[0] == node.children[1];
                break;
            }
            case OpType::NotEqual:
            {
                eval = node.children[0] != node.children[1];
                break;
            }
            case OpType::Or:
            {
                eval = node.children[0] || node.children[1];
                break;
            }
            case OpType::And:
            {
                eval = node.children[0] && node.children[1];
                break;
            }
            case OpType::GreaterThan:
            {
                eval = node.children[0] > node.children[1];
                break;
            }
            case OpType::GreaterThanOrEqual:
            {
                eval = node.children[0] >= node.children[1];
                break;
            }
            case OpType::LessThan:
            {
                eval = node.children[0] < node.children[1];
                break;
            }
            case OpType::LessThanOrEqual:
            {
                eval = node.children[0] <= node.children[1];
                break;
            }
        }

        node.type = Type::Bool;
        node.bool_value = eval;

        return {};
    }

}

bool evaluate_condition(const Scope& scope, const std::string& predicate)
{
    std::vector<EvalNode> nodes;
    tokenize(predicate, scope, nodes);
    auto tree = create_tree(nodes);
    if (tree.type == Type::None)
    {
        return false;
    }

    auto result = resolve_tree(tree);
    if (!result.has_value())
    {
        return false;
    }

    return tree.bool_value;
}

}
