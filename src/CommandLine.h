#pragma once
#include <string>

struct CommandLineArgs
{
    std::string user_provided_script;
    std::string chosen_template;
};

namespace CommandLine
{
    bool process_args(int argc, char* argv[], CommandLineArgs& args);
}
