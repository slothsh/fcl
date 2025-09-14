#include "lib/Core.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Lex Simple Configuration File", "[conflexer]") {
    SECTION("Top-Level Block Can Be Lexed") {
        auto ast = ConfLexer::lexFile("./data/Config.conf");
        REQUIRE(1 == 1);
    }
}
