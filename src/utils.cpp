#include "utils.h"

#include <algorithm>

namespace ekt
{
std::string utils::to_upper(const std::string& in)
{
    std::string upper = in;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    return upper;
}
}