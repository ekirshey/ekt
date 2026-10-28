#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "utils.h"
#include "ekt_defines.h"

#include <string>

using namespace ekt;

namespace
{
    std::string key(const std::string& value)
    {
        return std::format("{}{}{}", key_template_start, value, key_template_end);
    }

    std::string conditional(const std::string& label, const std::string& value)
    {
        return std::format("{}{}{}{}{}", conditional_template_start, label, pred_delimiter, value, conditional_template_end);
    }
}

TEST_CASE("an empty template string returns nullopt", "[utils]")
{
    REQUIRE(utils::find_template_end("", 0, key_template_start, key_template_end) == std::nullopt);
}

TEST_CASE("a well formed key template returns correct end position", "[utils]")
{
    const std::string body = "test";
    const std::string template_str = key(body);
    const size_t target = key_template_start.size() + body.size();

    REQUIRE(utils::find_template_end(template_str, 0, key_template_start, key_template_end) == target);
}

TEST_CASE("open with no close returns nullpot", "[utils]")
{
    const std::string body = key_template_start + "test";

    REQUIRE(utils::find_template_end(body, 0, key_template_start, key_template_end) == std::nullopt);
}

TEST_CASE("close at beginning with no open returns nullpot", "[utils]")
{
    const std::string body = key_template_end + "test";

    REQUIRE(utils::find_template_end(body, 0, key_template_start, key_template_end) == std::nullopt);
}

TEST_CASE("close at endwith no open returns nullpot", "[utils]")
{
    const std::string body = "test" + key_template_end;

    REQUIRE(utils::find_template_end(body, 0, key_template_start, key_template_end) == std::nullopt);
}

TEST_CASE("correct templates in malformed string returns nullpot", "[utils]")
{
    const std::string body = key_template_start + key_template_start + "test" + key_template_end;

    REQUIRE(utils::find_template_end(body, 0, key_template_start, key_template_end) == std::nullopt);
}

TEST_CASE("string with no templates returns nullopt", "[utils]")
{
    const std::string body = "test";

    REQUIRE(utils::find_template_end(body, 0, key_template_start, key_template_end) == std::nullopt);
}

TEST_CASE("returns first end when multiple pairs exist", "[utils]")
{
    const std::string body1 = "test";
    const std::string t = key(body1) + key("test2");

    const auto target = key_template_start.size() + body1.size();
    REQUIRE(utils::find_template_end(t, 0, key_template_start, key_template_end) == target);
}

TEST_CASE("nested key template returns correct end position", "[utils]")
{
    const std::string inner_template_str = key("body");
    const std::string outer_template_str  = key(inner_template_str);

    const size_t target = key_template_start.size() + inner_template_str.size();

    REQUIRE(utils::find_template_end(outer_template_str, 0, key_template_start, key_template_end) == target);
}

TEST_CASE("deeply nested key template returns correct end position", "[utils]")
{
    std::string t = "test";
    for(int i = 0; i < 100; i++)
    {
        t = key(t);
    }
    const std::string outer_template_str = key(t);
    const size_t target = key_template_start.size() + t.size();

    REQUIRE(utils::find_template_end(outer_template_str, 0, key_template_start, key_template_end) == target);
}

TEST_CASE("conditional template returns correct end position", "[utils]")
{
    const std::string label = "erik";
    const std::string body = "body";
    const size_t target = conditional_template_start.size() + label.size() + pred_delimiter.size() + body.size();
    REQUIRE(utils::find_template_end(conditional(label, body), 0, conditional_template_start, conditional_template_end) == target);
}

TEST_CASE("conditional template with nested key returns correct end position", "[utils]")
{
    const std::string label = "erik";
    const std::string body = key("body");
    const size_t target = conditional_template_start.size() + label.size() + pred_delimiter.size() + body.size();
    REQUIRE(utils::find_template_end(conditional(label, body), 0, conditional_template_start, conditional_template_end) == target);
}

TEST_CASE("conditional template with nested conditional returns correct end position", "[utils]")
{
    const std::string label = "erik";
    const std::string body = conditional("nested", "body");
    const size_t target = conditional_template_start.size() + label.size() + pred_delimiter.size() + body.size();
    REQUIRE(utils::find_template_end(conditional(label, body), 0, conditional_template_start, conditional_template_end) == target);
}

TEST_CASE("loop template with nested loop returns correct end position", "[utils]")
{
    const std::string body = loop_template_start + "inner" + loop_template_end + " after";
    const std::string template_str = loop_template_start + body + loop_template_end;
    const size_t target = loop_template_start.size() + body.size();
    REQUIRE(utils::find_template_end(template_str, 0, loop_template_start, loop_template_end) == target);
}

TEST_CASE("nested templates step past the given end delimiter, not the key end delimiter", "[utils]")
{
    // Delimiters shorter than key_template_end would skip characters if the wrong length were used
    REQUIRE(utils::find_template_end("(())", 0, "(", ")") == 3);
    REQUIRE(utils::find_template_end("(()())", 0, "(", ")") == 5);
    REQUIRE(utils::find_template_end("((x)y)z", 0, "(", ")") == 5);
}
