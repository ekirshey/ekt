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
        FuncReturnVal is_set(const std::vector<FuncArg>& args, const Scope& scope)
        {
            if (args.size() != 1 || args[0].type != FuncArgType::Literal)
            {
                return ret_val(false);
            }

            return ret_val(!scope.get_value(args[0].string_value).empty());
        }

        FuncReturnVal is_arr_empty(const std::vector<FuncArg>& args, const Scope& scope)
        {
            if (args.size() != 1 || args[0].type != FuncArgType::Literal)
            {
                return ret_val(false);
            }

            return ret_val(scope.get_array(args[0].string_value).empty());
        }

        FuncReturnVal get_index(const std::vector<FuncArg>& args, const Scope& scope)
        {
            if (args.size() != 1 || args[0].type != FuncArgType::Literal)
            {
                return ret_val(false);
            }

            return ret_val(-1);
        }

        const FunctionDefinition is_set_def = {
            .arg_types = {FuncArgType::Literal},
            .impl = &is_set
        };

        const FunctionDefinition is_arr_empty_def = {
            .arg_types = {FuncArgType::Literal},
            .impl = &is_arr_empty
        };

        std::unordered_map<std::string, FunctionDefinition> functions = {
            {"IS_SET", is_set_def},
            {"IS_ARRAY_EMPTY", is_arr_empty_def},
            {"INDEX", get_index}
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
                            .string_value = std::string(trimmed)
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
