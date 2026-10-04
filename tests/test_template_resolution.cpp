#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "Context.h"
#include "ParsedTemplateString.h"

#include <string>

// These tests exercise the resolution engine directly: parse a template string
// and check that resolving it against a Context produces the expected output
// contents. test_lua_interface.cpp covers the same machinery end to end through
// the Lua bindings; here we pin down the substitution rules in isolation.

namespace
{
    // Builds a context from key/value pairs. Context upper-cases keys on insert,
    // and ParsedTemplateString upper-cases the names it looks up, so callers can
    // spell keys however they like.
    Context make_context(std::initializer_list<std::pair<std::string, std::string>> pairs)
    {
        Context context;
        for (const auto& [key, value] : pairs)
        {
            context.insert(key, value);
        }
        return context;
    }

    // Parses content (failing the test if it does not parse) and resolves it.
    std::string resolve(const std::string& content, const Context& context)
    {
        auto parsed = ParsedTemplateString::parse(content);
        if (!parsed) { FAIL("Failed to parse: " + content); }
        return parsed->resolve(context);
    }
}

TEST_CASE("parse rejects empty content", "[resolution]")
{
    auto parsed = ParsedTemplateString::parse("");
    REQUIRE_FALSE(parsed.has_value());
}

TEST_CASE("content with no variables is returned verbatim", "[resolution]")
{
    Context context;

    SECTION("plain text")
    {
        REQUIRE(resolve("just some text", context) == "just some text");
    }

    SECTION("newlines and indentation are preserved")
    {
        const std::string content = "line one\n    indented\nline three\n";
        REQUIRE(resolve(content, context) == content);
    }
}

TEST_CASE("a single variable is substituted", "[resolution]")
{
    auto context = make_context({{"name", "ekt"}});

    SECTION("on its own")
    {
        REQUIRE(resolve("![[name]]", context) == "ekt");
    }

    SECTION("surrounded by literal text")
    {
        REQUIRE(resolve("hello ![[name]]!", context) == "hello ekt!");
    }
}

TEST_CASE("a variable appearing at the boundaries is handled", "[resolution]")
{
    auto context = make_context({{"a", "A"}, {"b", "B"}});

    SECTION("at the very start")
    {
        REQUIRE(resolve("![[a]] tail", context) == "A tail");
    }

    SECTION("at the very end")
    {
        REQUIRE(resolve("head ![[b]]", context) == "head B");
    }

    SECTION("adjacent variables with nothing between them")
    {
        REQUIRE(resolve("![[a]]![[b]]", context) == "AB");
    }
}

TEST_CASE("multiple and repeated variables all resolve", "[resolution]")
{
    auto context = make_context({{"first", "Erik"}, {"last", "Kirshey"}});

    SECTION("distinct variables")
    {
        REQUIRE(resolve("![[first]] ![[last]]", context) == "Erik Kirshey");
    }

    SECTION("the same variable used more than once")
    {
        REQUIRE(resolve("![[first]], ![[first]], ![[first]]", context) ==
                "Erik, Erik, Erik");
    }
}

TEST_CASE("variable lookups are case insensitive", "[resolution]")
{
    auto context = make_context({{"Project_Name", "ekt"}});

    // The template spells the name differently from how it was inserted; both
    // are upper-cased internally so they still match.
    REQUIRE(resolve("![[project_name]]", context) == "ekt");
    REQUIRE(resolve("![[PROJECT_NAME]]", context) == "ekt");
}

TEST_CASE("an unknown variable resolves to nothing", "[resolution]")
{
    auto context = make_context({{"known", "value"}});

    SECTION("the marker is dropped, surrounding text is kept")
    {
        REQUIRE(resolve("a![[missing]]b", context) == "ab");
    }

    SECTION("only the unknown variable is dropped")
    {
        REQUIRE(resolve("![[known]]/![[missing]]", context) == "value/");
    }
}

TEST_CASE("a marker with no closing ]] fails to parse", "[resolution]")
{
    // The expectation is correctly formed templates
    REQUIRE_FALSE(ParsedTemplateString::parse("value is ![[name"));
}

TEST_CASE("an opener within an opener fails to parse", "[resolution]")
{
    // No recursive variables
    REQUIRE_FALSE(ParsedTemplateString::parse("![[unterminated and ![[name]]"));
}

TEST_CASE("variables() reports the parsed markers", "[resolution]")
{
    auto parsed = ParsedTemplateString::parse("![[a]] and ![[b]] and ![[a]]");
    REQUIRE(parsed.has_value());

    const auto& vars = parsed->variables();
    REQUIRE(vars.size() == 3);

    // Names come back upper-cased regardless of how they appear in the source.
    REQUIRE(parsed->get_variable(vars[0]) == "A");
    REQUIRE(parsed->get_variable(vars[1]) == "B");
    REQUIRE(parsed->get_variable(vars[2]) == "A");
}
