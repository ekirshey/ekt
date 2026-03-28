#pragma once

#define TRY_UNWRAP(var, expr)          \
    const auto var##_opt = (expr);     \
    if (!var##_opt.has_value())        \
        return false;                  \
    const auto& var = *var##_opt
