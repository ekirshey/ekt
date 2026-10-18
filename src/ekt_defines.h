#pragma once

#include <string>

namespace ekt
{
    const std::string key_template_start = "[[!";
    const std::string key_template_end = "!]]";

    const std::string loop_template_start = "[[...";
    const std::string loop_template_end = "...]]";

    const std::string conditional_template_start = "[[==";
    const std::string conditional_template_end = "==]]";

    const std::string label_delimiter = "|";
    const std::string ownership_delimiter = ".";

}
