#pragma once
#include <string>

#include "Scope.h"
#include <EvalNode.h>

namespace ekt
{
bool evaluate_condition(const Scope& scope, const std::string& predicate);
std::optional<EvalNode> literal_to_node(const Scope& scope, std::string_view str);
}
