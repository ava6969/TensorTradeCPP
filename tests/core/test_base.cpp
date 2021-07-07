//
// Created by dewe on 7/3/21.
//

#include "catch.hpp"
#include "../../core/base.h"

TEST_CASE("Basic Identifiable init")
{
    ttc::Identifiable identifiable;

    std::cout << "generated uuid: " << identifiable.ID();

    identifiable.setID("adesola");

    REQUIRE(identifiable.ID() == "adesola");

}

