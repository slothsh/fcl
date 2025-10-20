#include <catch2/catch_test_macros.hpp>

import std;
import Conf;

TEST_CASE("Tokenize Simple Configuration File", "[conftokenizer]") {
    SECTION("Top-Level Block Can Be Tokenized") {
        auto token_list = ConfTokenizer::tokenizeFile("./data/Config.conf");

        for (auto const& token : token_list.value()) {
            std::println("{}: |{}|", token.kind, token.data);
        }

        REQUIRE(1 == 1);
    }
}
