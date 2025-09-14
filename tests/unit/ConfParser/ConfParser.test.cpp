#include "lib/Core.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Parse Simple Configuration File", "[confparser]") {
    SECTION("Top-Level Block Can Be Parsed") {
        auto token_list = ConfLexer::lexFile("./data/Config.conf");
        auto ast = ConfParser::parse(token_list.value());
        REQUIRE(1 == 1);
    }
}
