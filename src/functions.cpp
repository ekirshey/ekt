#include "functions.h"
#include <string_view>
#include <ranges>

#include "utils.h"

namespace ekt
{
    namespace {
        FuncReturnVal ret_val(bool val)
        {
            return FuncReturnVal{
                .type = FuncReturnType::Bool,
                .bool_value = val
            };
        }

        FuncReturnVal ret_val(int val)
        {
            return FuncReturnVal{
                .type = FuncReturnType::Number,
                .number_value = val
            };
        }

        // Function definitions
        bool is_set(const std::vector<FuncArg>& args, const Scope& scope)
        {
            if (args.size() != 1 || args[0].type != FuncArgType::Literal)
            {
                return false;
            }

            return !scope.get_value(args[0].literal_value).empty();
        }

        bool is_arr_empty(const std::vector<FuncArg>& args, const Scope& scope)
        {
            if (args.size() != 1 || args[0].type != FuncArgType::Literal)
            {
                return false;
            }

            return scope.get_array(args[0].literal_value).empty();
        }

        int get_index(const std::vector<FuncArg>& args, const Scope& scope)
        {
            if (args.size() != 1 || args[0].type != FuncArgType::Literal)
            {
                return -1;
            }

            return scope.get_num_child_contexts(args[0].literal_value) - 1;
        }

        bool if_any(const std::vector<FuncArg>& args, const Scope& scope)
        {
            if (args.size() != 2 || args[0].type != FuncArgType::Literal)
            {
                return false;
            }

            std::string parent, field;
            auto result = utils::split_key(args[0].literal_value, parent, field);
            if (!result)
            {
                return false;
            }

            auto array = scope.get_array(parent);
            if (array.empty())
            {
                return false;
            }

            for(auto& a : array)
            {
                if (a.get_value(field) == args[1].literal_value)
                {
                    return true;
                }
            }

            return false;
        }

        const FunctionDefinition is_set_def = {
            .arg_types = {FuncArgType::Literal},
            .impl = [](const std::vector<FuncArg>& args, const Scope& scope) {return ret_val(is_set(args, scope));}
        };

        const FunctionDefinition is_arr_empty_def = {
            .arg_types = {FuncArgType::Literal},
            .impl = [](const std::vector<FuncArg>& args, const Scope& scope) {return ret_val(is_arr_empty(args, scope));}
        };

        const FunctionDefinition get_index_def = {
            .arg_types = {FuncArgType::Literal},
            .impl = [](const std::vector<FuncArg>& args, const Scope& scope) {return ret_val(get_index(args, scope));}
        };

        const FunctionDefinition if_any_def = {
            .arg_types = {FuncArgType::Literal, FuncArgType::Literal},
            .impl = [](const std::vector<FuncArg>& args, const Scope& scope) {return ret_val(if_any(args, scope));}
        };

        std::unordered_map<std::string, FunctionDefinition> functions = {
            {"IS_SET", is_set_def},
            {"IS_ARRAY_EMPTY", is_arr_empty_def},
            {"INDEX", get_index_def},
            {"IF_ANY", if_any_def},
        };
    }

    std::optional<Function> is_function(const std::string_view& predicate)
    {
        auto split = std::views::split(predicate, std::string_view("("))
            | std::ranges::to<std::vector<std::string>>();

        if (split.size() != 2)
        {
            return std::nullopt;
        }

        if (split[1].back() != ')')
        {
            return std::nullopt;
        }

        auto args_str = split[1].substr(0, split[1].size() -1 );

        auto func_it = functions.find(utils::to_upper(split[0]));
        if (func_it == functions.end())
        {
            return std::nullopt;
        }

        auto func = func_it->second;

        if (func.arg_types.empty())
        {
            if(!args_str.empty())
            {
                return std::nullopt;
            }

            return Function{
                .args = {},
                .def = func
            };
        }
        else
        {
            auto arg_split = std::views::split(split[1].substr(0, split[1].size() - 1 ), std::string_view(","))
                | std::ranges::to<std::vector<std::string>>();

            if (arg_split.size() != func.arg_types.size())
            {
                return std::nullopt;
            }

            Function new_func;
            new_func.def = func;
            int i = 0;
            for(auto& a : func.arg_types)
            {
                auto trimmed = utils::trim(arg_split[i]);
                switch(a)
                {
                    case FuncArgType::Literal:
                    {
                        new_func.args.push_back(FuncArg{
                            .type = a,
                            .literal_value = std::string(trimmed)
                        });
                        break;
                    }
                    case FuncArgType::Number:
                    {
                        auto num = utils::to_int(trimmed);
                        if (num)
                        {
                            new_func.args.push_back(FuncArg{
                                .type = a,
                                .number_value = num.value()
                            });
                        }
                        else
                        {
                            return std::nullopt;
                        }
                    }
                }

                i++;
            }

            return new_func;
        }

        return std::nullopt;
    }
}
