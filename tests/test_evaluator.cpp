#include <catch2/catch_test_macros.hpp>

#include "Context.h"
#include "Scope.h"
#include "evaluator.h"

#include <string>

// These tests pin down how conditional predicates are evaluated: comparisons
// (==, !=) between variables and quoted literals, and combining comparisons
// with && and ||.

using namespace ekt;

namespace
{
    // Context holding the values the predicates below compare against
    Context make_context()
    {
        Context context;
        context.set("A", "true");
        context.set("B", "false");
        context.set("C", "true");
        context.set("NUM", "1");
        context.set("BIG_NUM", "100");
        context.set("NAME", "erik");
        context.set("OTHER_NAME", "erik");

        Context item;
        item.set("VALUE", "1");
        context.append("ITEMS", item);
        context.append("ITEMS", item);
        context.append("ITEMS", item);
        context.append("ITEMS", item);
        return context;
    }

    bool evaluate(const std::string& predicate)
    {
        const Context context = make_context();
        Scope scope;
        scope.set_root_context(&context);
        return evaluate_condition(scope, predicate);
    }
}

TEST_CASE("index returns current loop index", "[evaluator]")
{
    const Context context = make_context();
    Scope scope;
    scope.set_root_context(&context);

    auto arr = scope.get_array("ITEMS");

    REQUIRE(evaluate_condition(scope, "INDEX(ITEMS) == -1"));

    scope.add_child_context("ITEMS", &arr[0]);
    REQUIRE(evaluate_condition(scope, "INDEX(ITEMS) == 0"));
    REQUIRE(evaluate_condition(scope, "INDEX(ITEMS) >= 0"));
    REQUIRE_FALSE(evaluate_condition(scope, "INDEX(ITEMS) == 1"));

    scope.add_child_context("ITEMS", &arr[1]);
    REQUIRE(evaluate_condition(scope, "INDEX(ITEMS) == 1"));
    REQUIRE(evaluate_condition(scope, "INDEX(ITEMS) > 0"));
    REQUIRE(evaluate_condition(scope, "INDEX(ITEMS) < 2"));
    REQUIRE(evaluate_condition(scope, "INDEX(ITEMS) <= 1"));
    REQUIRE(evaluate_condition(scope, "INDEX(ITEMS) == NUM"));
    REQUIRE(evaluate_condition(scope, "INDEX(ITEMS) == \"1\""));

    REQUIRE_FALSE(evaluate_condition(scope, "INDEX(ITEMS) == A"));
}

TEST_CASE("number comparisons", "[evaluator]")
{
    REQUIRE(evaluate("NUM == 1"));
    REQUIRE_FALSE(evaluate("NUM == \"yo\""));
    REQUIRE(evaluate("NUM == \"1\""));
    REQUIRE(evaluate("NUM < 100"));
    REQUIRE(evaluate("NUM <= 1"));
    REQUIRE(evaluate("BIG_NUM > NUM"));
    REQUIRE_FALSE(evaluate("BIG_NUM < NUM"));
    REQUIRE(evaluate("BIG_NUM >= 100"));
}

TEST_CASE("is_array_empty evaluates to false if array is not empty", "[evaluator]")
{
    REQUIRE_FALSE(evaluate("is_array_empty(ITEMS)"));
    REQUIRE(evaluate("is_array_empty(sdfdsf)"));
    REQUIRE(evaluate("is_array_empty(A)"));
    REQUIRE_FALSE(evaluate("is_array_empty(items)"));
    REQUIRE(evaluate("is_array_empty(ITEMS) || is_array_empty(sdfdsf)"));
    REQUIRE_FALSE(evaluate("is_array_empty(ITEMS) && is_array_empty(sdfdsf)"));
    REQUIRE(evaluate("is_array_empty(ITEMS) || A == \"true\""));
    REQUIRE_FALSE(evaluate("is_array_empty(ITEMS) && A == \"true\""));
}

TEST_CASE("is_set evaluates to true if a value exists", "[evaluator]")
{
    REQUIRE(evaluate("is_set(A)"));
    REQUIRE_FALSE(evaluate("is_set(sdfdsf)"));
    REQUIRE(evaluate("is_set(sdfdsf) || is_set(A)"));
    REQUIRE_FALSE(evaluate("is_set(sdfdsf) && is_set(A)"));
    REQUIRE(evaluate("is_set(sdfdsf) || A == \"true\""));
    REQUIRE_FALSE(evaluate("is_set(sdfdsf) && A == \"true\""));
}

TEST_CASE("== compares a variable to a literal", "[evaluator]")
{
    REQUIRE(evaluate("A == \"true\""));
    REQUIRE_FALSE(evaluate("A == \"false\""));
    REQUIRE(evaluate("NAME == \"erik\""));
    REQUIRE_FALSE(evaluate("NAME == \"bob\""));
}

TEST_CASE("== compares a literal to a variable", "[evaluator]")
{
    REQUIRE(evaluate("\"true\" == A"));
    REQUIRE_FALSE(evaluate("\"true\" == B"));
}

TEST_CASE("== compares two variables", "[evaluator]")
{
    REQUIRE(evaluate("NAME == OTHER_NAME"));
    REQUIRE(evaluate("A == C"));
    REQUIRE_FALSE(evaluate("A == B"));
}

TEST_CASE("== compares two literals", "[evaluator]")
{
    REQUIRE(evaluate("\"x\" == \"x\""));
    REQUIRE_FALSE(evaluate("\"x\" == \"y\""));
}

TEST_CASE("!= compares a variable to a literal", "[evaluator]")
{
    REQUIRE(evaluate("A != \"false\""));
    REQUIRE_FALSE(evaluate("A != \"true\""));
}

TEST_CASE("!= compares two variables", "[evaluator]")
{
    REQUIRE(evaluate("A != B"));
    REQUIRE_FALSE(evaluate("A != C"));
}

TEST_CASE("an unknown variable compares as empty", "[evaluator]")
{
    REQUIRE(evaluate("MISSING == \"\""));
    REQUIRE_FALSE(evaluate("MISSING == \"true\""));
    REQUIRE(evaluate("MISSING != \"true\""));
}

TEST_CASE("variable lookups are case insensitive", "[evaluator]")
{
    REQUIRE(evaluate("name == \"erik\""));
}

TEST_CASE("literals are case sensitive", "[evaluator]")
{
    REQUIRE_FALSE(evaluate("NAME == \"ERIK\""));
}

TEST_CASE("whitespace around operands is ignored", "[evaluator]")
{
    REQUIRE(evaluate("A==\"true\""));
    REQUIRE(evaluate("  A   ==   \"true\"  "));
    REQUIRE(evaluate("\tA\t==\t\"true\"\t"));
}

TEST_CASE("whitespace inside a literal is kept", "[evaluator]")
{
    REQUIRE_FALSE(evaluate("NAME == \" erik \""));
}

TEST_CASE("&& is true only when both sides are true", "[evaluator]")
{
    REQUIRE(evaluate("A == \"true\" && C == \"true\""));
    REQUIRE_FALSE(evaluate("A == \"true\" && B == \"true\""));
    REQUIRE_FALSE(evaluate("B == \"true\" && A == \"true\""));
    REQUIRE_FALSE(evaluate("B == \"true\" && B == \"x\""));
}

TEST_CASE("|| is true when either side is true", "[evaluator]")
{
    REQUIRE(evaluate("A == \"true\" || C == \"true\""));
    REQUIRE(evaluate("A == \"true\" || B == \"true\""));
    REQUIRE(evaluate("B == \"true\" || A == \"true\""));
    REQUIRE_FALSE(evaluate("B == \"true\" || B == \"x\""));
}

TEST_CASE("&& and || work with != comparisons", "[evaluator]")
{
    REQUIRE(evaluate("A != \"false\" && B != \"true\""));
    REQUIRE(evaluate("A != \"true\" || B != \"true\""));
    REQUIRE_FALSE(evaluate("A != \"true\" || B != \"false\""));
}

TEST_CASE("chains of the same logical operator", "[evaluator]")
{
    REQUIRE(evaluate("A == \"true\" && C == \"true\" && NAME == \"erik\""));
    REQUIRE_FALSE(evaluate("A == \"true\" && C == \"true\" && B == \"true\""));
    REQUIRE(evaluate("B == \"true\" || B == \"x\" || A == \"true\""));
    REQUIRE_FALSE(evaluate("B == \"true\" || B == \"x\" || A == \"x\""));
}

// Only cases where && and || precedence doesn't change the result, since the
// evaluator doesn't define one yet
TEST_CASE("mixing && and ||", "[evaluator]")
{
    REQUIRE(evaluate("A == \"true\" && C == \"true\" || B == \"true\""));
    REQUIRE(evaluate("B == \"true\" || A == \"true\" && C == \"true\""));
    REQUIRE_FALSE(evaluate("B == \"true\" && A == \"true\" || B == \"x\""));
    REQUIRE_FALSE(evaluate("B == \"x\" || B == \"true\" && A == \"true\""));
}

TEST_CASE("malformed predicates evaluate to false", "[evaluator]")
{
    SECTION("empty predicate")
    {
        REQUIRE_FALSE(evaluate(""));
    }

    SECTION("operator with no left operand")
    {
        REQUIRE_FALSE(evaluate("== \"true\""));
    }

    SECTION("operator with no right operand")
    {
        REQUIRE_FALSE(evaluate("A =="));
    }

    SECTION("logical operator with a missing comparison")
    {
        REQUIRE_FALSE(evaluate("A == \"true\" &&"));
        REQUIRE_FALSE(evaluate("|| A == \"true\""));
    }

    SECTION("logical operator with a missing comparison")
    {
        REQUIRE_FALSE(evaluate("A == \"true\" &&"));
        REQUIRE_FALSE(evaluate("|| A == \"true\""));
    }

    SECTION("logical operator between bare values")
    {
        REQUIRE_FALSE(evaluate("A && C"));
    }

    SECTION("only a single logical operator")
    {
        REQUIRE_FALSE(evaluate("&&"));
    }

    SECTION("multiplelogical operators")
    {
        REQUIRE_FALSE(evaluate("|| && ||"));
    }
}
