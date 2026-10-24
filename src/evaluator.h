#pragma once
#include <string>

#include "Scope.h"

namespace ekt
{
bool evaluate_condition(const Scope& scope, const std::string& predicate);
}
