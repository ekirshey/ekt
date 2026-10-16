#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "Context.h"
#include "ParsedTemplateString.h"
#include "ekt_defines.h"

#include <format>
#include <string>

// These tests exercise the resolution engine directly: parse a template string
// and check that resolving it against a Context produces the expected output
// contents. test_lua_interface.cpp covers the same machinery end to end through
// the Lua bindings; here we pin down the substitution rules in isolation.


using namespace ekt;

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
            context.set(key, value);
        }
        return context;
    }

    // Parses content (failing the test if it does not parse) and resolves it.
    std::string resolve(const std::string& content, const Context& context)
    {
        auto parsed = ParsedTemplateString::parse(content);
        if (!parsed) 
        { 
            FAIL("Failed to parse: " + content); 
        }
        return parsed->resolve(context);
    }

    std::string key(const std::string& value)
    {
        return std::format("{}{}{}", key_template_start, value, key_template_end);
    }

    std::string conditional(const std::string& label, const std::string& value)
    {
        return std::format("{}{}{}{}{}", conditional_template_start, label, label_delimiter, value, conditional_template_end);
    }

    std::string loop(const std::string& label, const std::string& value)
    {
        return std::format("{}{}{}{}{}", loop_template_start, label, label_delimiter, value, loop_template_end);
    }
}

TEST_CASE("parse allows empty content", "[resolution]")
{
    auto parsed = ParsedTemplateString::parse("");
    REQUIRE(parsed.has_value());
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
        REQUIRE(resolve(key("name"), context) == "ekt");
    }

    SECTION("surrounded by literal text")
    {
        REQUIRE(resolve(std::format("hello {}!", key("name")), context) == "hello ekt!");
    }
}

TEST_CASE("a variable appearing at the boundaries is handled", "[resolution]")
{
    auto context = make_context({{"a", "A"}, {"b", "B"}});

    SECTION("at the very start")
    {
        REQUIRE(resolve(std::format("{} tail", key("a")), context) == "A tail");
    }

    SECTION("at the very end")
    {
        REQUIRE(resolve(std::format("head {}", key("b")), context) == "head B");
    }

    SECTION("adjacent variables with nothing between them")
    {
        REQUIRE(resolve(std::format("{}{}", key("a"), key("b")), context) == "AB");
    }
}

TEST_CASE("multiple and repeated variables all resolve", "[resolution]")
{
    auto context = make_context({{"first", "Erik"}, {"last", "Kirshey"}});

    SECTION("distinct variables")
    {
        REQUIRE(resolve(std::format("{} {}", key("first"), key("last")), context) == "Erik Kirshey");
    }

    SECTION("the same variable used more than once")
    {
        REQUIRE(resolve(std::format("{0}, {0}, {0}", key("first")), context) ==
                "Erik, Erik, Erik");
    }
}

TEST_CASE("cursive variables all resolve", "[resolution]")
{
    auto context = make_context({{"Kirshey", "Erik"}, {"last", "Kirshey"}});

    SECTION("one level deep")
    {
        REQUIRE(resolve(key(key("last")), context) == "Erik");
    }

}
TEST_CASE("variable lookups are case insensitive", "[resolution]")
{
    auto context = make_context({{"Project_Name", "ekt"}});

    // The template spells the name differently from how it was inserted; both
    // are upper-cased internally so they still match.
    REQUIRE(resolve(key("project_name"), context) == "ekt");
    REQUIRE(resolve(key("PROJECT_NAME"), context) == "ekt");
}

TEST_CASE("an unknown variable resolves to nothing", "[resolution]")
{
    auto context = make_context({{"known", "value"}});

    SECTION("the marker is dropped, surrounding text is kept")
    {
        REQUIRE(resolve(std::format("a{}b", key("missing")), context) == "ab");
    }

    SECTION("only the unknown variable is dropped")
    {
        REQUIRE(resolve(std::format("{}/{}", key("known"), key("missing")), context) == "value/");
    }
}

TEST_CASE("a marker with no closing  !]] fails to parse", "[resolution]")
{
    // The expectation is correctly formed templates
    REQUIRE_FALSE(ParsedTemplateString::parse(std::format("value is {}name", key_template_start)));
}

TEST_CASE("an opener within an opener fails to parse", "[resolution]")
{
    REQUIRE_FALSE(ParsedTemplateString::parse(std::format("{}unterminated and {}", key_template_start, key("name"))));
}

TEST_CASE("recursive key variables resolve", "[resolution]")
{
    REQUIRE(ParsedTemplateString::parse(key(key("test"))));
}

TEST_CASE("variables() reports the parsed markers", "[resolution]")
{
    auto parsed = ParsedTemplateString::parse(std::format("{0} and {1} and {0}", key("a"), key("b")));
    REQUIRE(parsed.has_value());

    const auto& vars = parsed->variables();
    REQUIRE(vars.size() == 3);

    // Names come back upper-cased regardless of how they appear in the source.
    REQUIRE(parsed->get_variable(vars[0]) == "A");
    REQUIRE(parsed->get_variable(vars[1]) == "B");
    REQUIRE(parsed->get_variable(vars[2]) == "A");
}

TEST_CASE("conditional with true condition parses correctly", "[resolution]")
{
    REQUIRE(ParsedTemplateString::parse(conditional("yo", "hi")));
}

TEST_CASE("conditional with true condition resolves to inner text", "[resolution]")
{
    Context context;
    context.set_condition("yo", true);
    context.set("key", "test");

    SECTION("resolves plain text")
    {
        auto parsed = ParsedTemplateString::parse(conditional("yo", "hi"));
        REQUIRE(parsed->resolve(context) == "hi");
    }

    SECTION("resolves inner key")
    {
        auto parsed = ParsedTemplateString::parse(conditional("yo", key("key")));
        REQUIRE(parsed->resolve(context) == "test");
    }
}

TEST_CASE("conditional with false condition resolves to empty text", "[resolution]")
{
    Context context;
    context.set_condition("yo", false);
    context.set("key", "test");

    SECTION("empty plain text")
    {
        auto parsed = ParsedTemplateString::parse(conditional("yo", "hi"));
        REQUIRE(parsed->resolve(context) == "");
    }

    SECTION("empty inner key")
    {
        auto parsed = ParsedTemplateString::parse(conditional("yo", key("key")));
        REQUIRE(parsed->resolve(context) == "");
    }
}

TEST_CASE("loop parses correctly", "[resolution]")
{
    REQUIRE(ParsedTemplateString::parse(loop("yo", "hi")));
}

TEST_CASE("loop with no elements resolves to empty string", "[resolution]")
{
    Context context;
    auto parsed = ParsedTemplateString::parse(loop("yo", "hi"));
    REQUIRE(parsed->resolve(context) == "");
}

TEST_CASE("loop with plain text resolves to text n times", "[resolution]")
{
    Context context;
    context.append("yo", {});
    context.append("yo", {});
    context.append("yo", {});
    context.append("yo", {});
    context.append("yo", {});

    auto parsed = ParsedTemplateString::parse(loop("yo", "hi\n"));
    REQUIRE(parsed->resolve(context) == "hi\nhi\nhi\nhi\nhi\n");
}

TEST_CASE("loop with child variables resolve from the childs context", "[resolution]")
{
    Context context;
    context.append("yo", make_context({{"yo.hi", "0"}}));
    context.append("yo", make_context({{"yo.hi", "1"}}));
    context.append("yo", make_context({{"yo.hi", "2"}}));
    context.append("yo", make_context({{"yo.hi", "3"}}));
    context.append("yo", make_context({{"yo.hi", "4"}}));

    auto parsed = ParsedTemplateString::parse(loop("yo", key("yo.hi") + "\n"));
    REQUIRE(parsed->resolve(context) == "0\n1\n2\n3\n4\n");
}