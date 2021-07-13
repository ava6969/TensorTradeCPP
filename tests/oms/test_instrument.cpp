//
// Created by dewe on 7/4/21.
//

#include <include/oms/exchanges/exchange.h>
#include "catch.hpp"
#include "include/oms/instruments/exchange_pair.h"
#include "include/oms/instruments/trading_pair.h"
#include "include/core/dexceptions.h"
using namespace ttc;

TEST_CASE("Mock Exchange")
{

    SECTION("Valid Init")
    {
        Exchange bitfinex("bitfinex");
        ExchangePair exchangePair(bitfinex.Name(), USD/BTC);
        REQUIRE(exchangePair.pair().base() == USD);
        REQUIRE(exchangePair.pair().quote() == BTC);
    }

    SECTION("String")
    {
        Exchange bitfinex("bitfinex");
        ExchangePair exchangePair(bitfinex.Name(), USD/BTC);
        REQUIRE(exchangePair.str() == "bitfinex:USD/BTC");
    }

}

TEST_CASE("Instrument")
{
    SECTION("Init")
    {
        REQUIRE(BTC.symbol() == "BTC");
        REQUIRE(BTC.precision() == 8);
        REQUIRE(BTC.name() == "Bitcoin");
    }

    SECTION("Valid Equals AND not equals")
    {
        auto BTC2 = Instrument{"BTC", 8, "Bitcoin"};
        auto ETH2 = Instrument{"ETH", 8, "Bitcoin"};
        REQUIRE(BTC == BTC2);
        REQUIRE(BTC != ETH2);
    }

    SECTION("Right Multiply")
    {
        auto q = BTC * 8;
        auto q1 = 8 * BTC;

        REQUIRE(q.size == 8);
        REQUIRE(q.instrument == BTC);
    }

    SECTION("Divide")
    {
        auto pair = BTC/ETH;
        REQUIRE(pair.base() == BTC);
        REQUIRE(pair.quote() == ETH);

        REQUIRE_THROWS_AS(BTC/ BTC, ttc::InvalidTradingPair);
    }

    SECTION("str")
    {
        REQUIRE(BTC.str() == "BTC");
    }

}

TEST_CASE("Quantity")
{

    static string path_id = "f4cfeeae-a3e4-42e9-84b9-a24ccd2eebeb";
    static string other_id = "7f3de243-0474-48d9-bf44-ca55ae07a70e";
    static  Quantity q2(BTC, 10000, path_id);
    static  Quantity q(BTC, 10000);

    SECTION("Valid Init")
    {
        REQUIRE(q2.instrument == BTC);
        REQUIRE(q2.size == 10000);
        REQUIRE(q2.path_id == path_id);
    }

    SECTION("Valid Init")
    {
        REQUIRE_THROWS_AS((Quantity{BTC, -10000, path_id}), InvalidNegativeQuantity);
    }

    SECTION("Locking")
    {
        REQUIRE_FALSE(q.is_locked());
        REQUIRE(q2.is_locked());

        q.path_id = path_id;
        REQUIRE(q.is_locked());

        auto q3 = Quantity{BTC, 10000}.lock_for(path_id);
        REQUIRE(q3.is_locked());

    }

    SECTION("Valid Add")
    {
        q = Quantity{BTC, 10000} + Quantity{BTC, 500};
        REQUIRE(q.size == 10500);
        REQUIRE(q.instrument == BTC);

        q = Quantity{BTC, 10000, path_id} + Quantity{BTC, 500};
        REQUIRE(q.size == 10500);
        REQUIRE(q.instrument == BTC);
        REQUIRE(q.path_id == path_id);

        q = Quantity{BTC, 10000, path_id} + Quantity{BTC, 500, path_id};
        REQUIRE(q.size == 10500);
        REQUIRE(q.instrument == BTC);
        REQUIRE(q.path_id == path_id);

        q = Quantity{BTC, 10000} + 500.f;
        REQUIRE(q.size == 10500);
        REQUIRE(q.instrument == BTC);

        REQUIRE_THROWS_AS((Quantity{BTC, 10000.f} + Quantity{ETH, 500.f}), IncompatibleInstrumentOperation<float>);
        REQUIRE_THROWS_AS((Quantity{BTC, 10000, path_id} + Quantity{BTC, 500, other_id}), QuantityOpPathMismatch);

        q = Quantity{BTC, 10000};
        q += Quantity{BTC, 500};
        REQUIRE(q.size == 10500);
        REQUIRE(q.instrument == BTC);

        q = Quantity{BTC, 10000, path_id};
        q += Quantity{BTC, 500};
        REQUIRE(q.size == 10500);
        REQUIRE(q.instrument == BTC);
        REQUIRE(q.path_id == path_id);

        q = Quantity{BTC, 10000, path_id};
        q += Quantity{BTC, 500, path_id};
        REQUIRE(q.size == 10500);
        REQUIRE(q.instrument == BTC);
        REQUIRE(q.path_id == path_id);

        q = Quantity{BTC, 10000};
        q += 500;
        REQUIRE(q.size == 10500);
        REQUIRE(q.instrument == BTC);

        q = Quantity{BTC, 10000};
        q += 500.f;
        REQUIRE(q.size == 10500.f);
        REQUIRE(q.instrument == BTC);
    }

    SECTION("Valid Sub")
    {
        q = Quantity{BTC, 1000} - Quantity{BTC, 500};
        REQUIRE(q.size == 500);
        REQUIRE(q.instrument == BTC);

        q = Quantity{BTC, 1000, path_id} - Quantity{BTC, 500};
        REQUIRE(q.size == 500);
        REQUIRE(q.instrument == BTC);
        REQUIRE(q.path_id == path_id);

        q = Quantity{BTC, 1000, path_id} - Quantity{BTC, 500, path_id};
        REQUIRE(q.size == 500);
        REQUIRE(q.instrument == BTC);
        REQUIRE(q.path_id == path_id);

        q = Quantity{BTC, 1000} - 500.f;
        REQUIRE(q.size == 500);
        REQUIRE(q.instrument == BTC);

        REQUIRE_THROWS_AS((Quantity{BTC, 500.f} - Quantity{BTC, 1500.f}), InvalidNegativeQuantity);

    }

    SECTION("Valid Mul")
    {
        q = Quantity{ETH, 50} * Quantity{ETH, 5};
        REQUIRE(q.size == 250);
        REQUIRE(q.instrument == ETH);
    }


    SECTION("Valid Less Than")
    {
        q = Quantity{ETH, 50};
        q2 = Quantity{ETH, 5};

        REQUIRE( (q2 < q));
        REQUIRE(not(q < q2));
    }

    SECTION("Valid Greater Than")
    {
        q = Quantity{ETH, 50};
        q2 = Quantity{ETH, 5};
        REQUIRE( (q > q2));
        REQUIRE((not(q2 > q)));
    }

    SECTION("Valid Equals")
    {
        q = Quantity{ETH, 5};
        q2 = Quantity{ETH, 5};
        REQUIRE( (q == q2));
        REQUIRE((q2 == q));
    }

//    SECTION("Valid Negate")
//    {
//        q = Quantity{ETH, 5};
//        auto neg_q = -1 * q;
//        REQUIRE( neg_q == -5);
//    }

    SECTION("Free")
    {
        q = Quantity{ETH, 5};
        auto free = q.free();
        REQUIRE(free.size == 5);
        REQUIRE(free.instrument == ETH);
        REQUIRE(not free.is_locked());

        q = Quantity{ETH, 5, "fake_id"};
        free = q.free();
        REQUIRE(free.size == 5);
        REQUIRE(free.instrument == ETH);
        REQUIRE(not free.is_locked());
    }

    SECTION("Exchange Pair")
    {
//        ExchangePair exchangePair(Exchange{"bitfinex"}, USD/BTC);
//
//        Quantity q6{USD, 1000.f};
//        auto converted = q6.convert(exchangePair);
//
//        REQUIRE(converted.size == 1000.f/9000.f);
//        REQUIRE(converted.instrument == BTC);
//
//        auto q7 = Quantity{BTC, 1.6};
//        converted = q7.convert(exchangePair);
//
//        REQUIRE(converted.size == 1.6 * 9000);
//        converted.instrument == USD;

    }

}