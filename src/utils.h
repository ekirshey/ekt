#pragma once

#include <string>

#define TRY_UNWRAP(var, expr)          \
    const auto var##_opt = (expr);     \
    if (!var##_opt.has_value())        \
        return false;                  \
    const auto& var = *var##_opt

std::string to_upper(const std::string& in);
