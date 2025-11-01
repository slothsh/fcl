#include <catch2/catch_test_macros.hpp>

import std;
import Containers;

TEST_CASE("StaticVector Container", "[staticvector]") {
    SECTION("StaticVector is iteratable") {
        StaticVector vector{ 1, 2, 3 };

        for (auto const v : vector) {
            std::println("{}", v);
        }
        
        REQUIRE(1 == 1);
    }
}
