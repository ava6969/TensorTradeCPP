//
// Created by dewe on 7/5/21.
//
#include <exchanges/exchange.h>
#include <orders/order_spec.h>
#include "catch.hpp"
#include "instruments/exchange_pair.h"
#include "instruments/trading_pair.h"
#include "../../core/dexceptions.h"
#include "wallets/wallet.h"

using namespace ttc;


TEST_CASE("Exchange")
{
    Exchange exchange("bitfinex");
    exchange.setID("fake_exchange_id");

    auto side = TradeSide::BUY;
    auto trade_type = TradeType::MARKET;

    auto opt = ExchangePair{exchange, USD / BTC};
    OrderSpec orderSpec;
    orderSpec.side  = side;
    orderSpec.type = trade_type;
    orderSpec.exchange_pair = &opt;

    REQUIRE(orderSpec.side == side);
    REQUIRE(orderSpec.type == trade_type);
    REQUIRE(*orderSpec.exchange_pair == ExchangePair{exchange, USD / BTC});
    REQUIRE(not orderSpec._criteria);

    orderSpec._criteria = [](const Order*, Exchange const &) { return true; };
    REQUIRE(orderSpec._criteria);

}

TEST_CASE("Create from Buy Order")
{
    Exchange exchange("bitfinex");
    exchange.setID("fake_exchange_id");

    Wallets wallets{Wallet{&exchange, 10000 * USD},
                    Wallet{&exchange, 2 * BTC}};

    Portfolio portfolio{USD, wallets, nullptr};

}
