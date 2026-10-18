#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "Context.h"

#include <string>

// These tests pin down the Context container on its own: how keys are
// normalized and validated, what get_value returns for present/missing keys,
// and how concat merges two contexts under each ResolveMode.

using namespace ekt;

TEST_CASE("an empty context has no values", "[context]")
{
    Context context;

    REQUIRE_FALSE(context.contains("NAME"));
    REQUIRE(context.get_value("NAME") == "");
}

TEST_CASE("set stores a value retrievable by get_value", "[context]")
{
    Context context;
    REQUIRE(context.set("NAME", "world"));

    REQUIRE(context.contains("NAME"));
    REQUIRE(context.get_value("NAME") == "world");
}

TEST_CASE("keys are normalized to upper case", "[context]")
{
    Context context;
    context.set("MixedCase", "value");

    SECTION("contains is case-insensitive")
    {
        REQUIRE(context.contains("mixedcase"));
        REQUIRE(context.contains("MIXEDCASE"));
        REQUIRE(context.contains("MixedCase"));
    }

    SECTION("get_value is case-insensitive")
    {
        REQUIRE(context.get_value("mixedcase") == "value");
        REQUIRE(context.get_value("MIXEDCASE") == "value");
        REQUIRE(context.get_value("MixedCase") == "value");
    }

    SECTION("differently cased keys refer to the same entry")
    {
        context.set("mixedcase", "replaced");
        REQUIRE(context.get_value("MIXEDCASE") == "replaced");
    }
}

TEST_CASE("set overwrites an existing value", "[context]")
{
    Context context;
    context.set("NAME", "first");
    context.set("NAME", "second");

    REQUIRE(context.get_value("NAME") == "second");
}

TEST_CASE("set accepts an empty value", "[context]")
{
    Context context;
    context.set("NAME", "");

    REQUIRE(context.contains("NAME"));
    REQUIRE(context.get_value("NAME") == "");
}

TEST_CASE("get_value returns empty for a missing key", "[context]")
{
    Context context;
    context.set("PRESENT", "value");

    REQUIRE(context.get_value("MISSING") == "");
    REQUIRE_FALSE(context.contains("MISSING"));
}

TEST_CASE("append creates an entry without a string value", "[context]")
{
    Context element;
    element.set("NAME", "hello");

    Context context;
    REQUIRE(context.append("headers", element));
    REQUIRE(context.append("headers", element));

    REQUIRE(context.contains("HEADERS"));
    REQUIRE(context.get_value("headers") == "");
}

TEST_CASE("empty keys are rejected", "[context]")
{
    Context context;

    SECTION("set")
    {
        REQUIRE_FALSE(context.set("", "value"));
    }

    SECTION("append")
    {
        REQUIRE_FALSE(context.append("", Context{}));
    }

    REQUIRE_FALSE(context.contains(""));
    REQUIRE(context.get_value("") == "");
}

TEST_CASE("a rejected key does not disturb existing entries", "[context]")
{
    Context context;
    context.set("NAME", "value");

    REQUIRE_FALSE(context.set("", "other"));
    REQUIRE(context.get_value("NAME") == "value");
}

TEST_CASE("concat merges another context's entries", "[context]")
{
    const auto mode = GENERATE(ResolveMode::Ignore, ResolveMode::Overwrite);

    Context context;
    context.set("A", "a");

    Context other;
    other.set("B", "b");
    other.set("C", "c");

    context.concat(other, mode);

    REQUIRE(context.get_value("A") == "a");
    REQUIRE(context.get_value("B") == "b");
    REQUIRE(context.get_value("C") == "c");

    SECTION("the source context is left unchanged")
    {
        REQUIRE_FALSE(other.contains("A"));
        REQUIRE(other.get_value("B") == "b");
    }
}

TEST_CASE("concat with ResolveMode::Ignore keeps existing keys", "[context]")
{
    Context context;
    context.set("SHARED", "original");

    Context other;
    other.set("SHARED", "incoming");

    context.concat(other, ResolveMode::Ignore);

    REQUIRE(context.get_value("SHARED") == "original");
}

TEST_CASE("concat with ResolveMode::Overwrite replaces existing keys", "[context]")
{
    Context context;
    context.set("SHARED", "original");
    context.set("MINE", "mine");

    Context other;
    other.set("SHARED", "incoming");

    context.concat(other, ResolveMode::Overwrite);

    REQUIRE(context.get_value("SHARED") == "incoming");
    REQUIRE(context.get_value("MINE") == "mine");
}

TEST_CASE("concat with ResolveMode::Overwrite replaces the whole entry", "[context]")
{
    Context context;
    context.set("SHARED", "original");

    // The incoming entry only has a condition, so its string value is empty.
    Context other;
    context.concat(other, ResolveMode::Overwrite);

    REQUIRE(context.contains("SHARED"));
    REQUIRE(context.get_value("SHARED") == "original");
}

TEST_CASE("concat with an empty context is a no-op", "[context]")
{
    const auto mode = GENERATE(ResolveMode::Ignore, ResolveMode::Overwrite);

    Context context;
    context.set("A", "a");

    context.concat(Context{}, mode);

    REQUIRE(context.get_value("A") == "a");
}

TEST_CASE("concat into an empty context copies everything", "[context]")
{
    const auto mode = GENERATE(ResolveMode::Ignore, ResolveMode::Overwrite);

    Context other;
    other.set("A", "a");

    Context context;
    context.concat(other, mode);

    REQUIRE(context.get_value("A") == "a");
}
