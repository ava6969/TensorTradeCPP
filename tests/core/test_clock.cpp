//
// Created by dewe on 7/3/21.
//

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "include/core/clock.h"
#include "include/core/base.h"

TEST_CASE("Basic Clock Init")
{
    ttc::Clock clk;

    REQUIRE(clk.Start() == 0);
    REQUIRE(clk.Step() == 0);
}

TEST_CASE("Basic Clock Increment")
{
    ttc::Clock clk;

    clk.increment();

    REQUIRE(clk.Step() == 1);
}

struct FirstExample: public ttc::TimeIndexed
{
    std::string msg;
};

struct SecondExample: public ttc::TimeIndexed
{
    std::string msg;
};

TEST_CASE("Time Indexed Init")
{
    FirstExample example1;
    SecondExample example2;

    ttc::Clock clk;

    example1._clock = &clk;
    example2._clock = &clk;

    REQUIRE(example1._clock->Start() == 0);
    REQUIRE(example1._clock->Step() == 0);

    REQUIRE(example2._clock->Start() == 0);
    REQUIRE(example2._clock->Step() == 0);

    REQUIRE(example1._clock == example2._clock);

}