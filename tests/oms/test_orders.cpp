//
// Created by dewe on 7/5/21.
//
#include <include/oms/exchanges/exchange.h>
#include <include/oms/orders/order_spec.h>
#include "catch.hpp"
#include "include/oms/instruments/exchange_pair.h"
#include "include/oms/instruments/trading_pair.h"
#include "include/core/dexceptions.h"
#include "include/oms/wallets/wallet.h"

using namespace ttc;


TEST_CASE("Exchange")
{
    Exchange exchange("bitfinex");
    exchange.setID("fake_exchange_id");

    auto side = TradeSide::BUY;
    auto trade_type = TradeType::MARKET;

    auto opt = ExchangePair{"bitfinex", USD / BTC};
    OrderSpec orderSpec(side, trade_type, opt, {});

    REQUIRE(orderSpec.side == side);
    REQUIRE(orderSpec.type == trade_type);
    REQUIRE(orderSpec.exchange_pair == ExchangePair{"bitfinex", USD / BTC});
    REQUIRE(not orderSpec._criteria);

    orderSpec._criteria = [](const Order &, Exchange const &) { return true; };
    REQUIRE(orderSpec._criteria);

}

TEST_CASE("Create from Buy Order")
{
    Exchange exchange("bitfinex");
    exchange.setID("fake_exchange_id");

    Wallet w1{"exchange", 2.f * BTC};
    Wallet w2{"bitfinex", 10000.f * USD};

    Portfolio portfolio{USD, {w1.ID(), w2.ID()}, nullptr};

}
