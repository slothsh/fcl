#include <catch2/catch_test_macros.hpp>
#include <print>

using namespace Conf;
using namespace Conf::Language;

TEST_CASE("Lex Simple Configuration File", "[conflexer]") {
    SECTION("Top-Level Block Can Be Lexed") {
        auto token_list = ConfTokenizer::lexFile("./data/Config.conf");

        for (auto const& token : token_list.value()) {
            std::println("{}: |{}|", token.kind, token.data);
        }

        REQUIRE(1 == 1);
    }
}
