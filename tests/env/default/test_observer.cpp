//
// Created by dewe on 7/9/21.
//

#include "catch2/catch.hpp"


#include "catch.hpp"
#include <boost/coroutine/all.hpp>
#include <iostream>
#include <include/oms/exchanges/exchange.h>
#include <include/env/default/observers.h>
#include <include/oms/wallets/ledger.h>
#include "include/oms/instruments/instrument.h"
#include "include/env/default/rewards.h"
#include "include/oms/wallets/wallet.h"

using namespace boost::coroutines;
using namespace ttc;


TEST_CASE("Price ds")
{
    auto btc_price = source<float>({7000, 7500, 8300}); btc_price->rename("USD-BTC");
    auto eth_price = source<float>({200, 212, 400}); eth_price->rename("USD-ETH");

    DataFeed feed({btc_price, eth_price}, false);
    auto res = feed.next<unordered_map<string, float>>();

    REQUIRE_NOTHROW(res["USD-BTC"] ==  7000);
    REQUIRE(res["USD-BTC"] ==  7000);
    REQUIRE_NOTHROW(res["USD-ETH"] ==  200);
    REQUIRE(res["USD-ETH"] ==  200);
}

TEST_CASE("Exchange Feed")
{
    auto btc_price = source<float>({7000, 7500, 8300}); btc_price->rename("USD-BTC");
    auto eth_price = source<float>({200, 212, 400}); eth_price->rename("USD-ETH");

    auto exchange = Exchange("bitfinex", Service{}, {}, {btc_price, eth_price});

    DataFeed feed(exchange.streams(), false);

    auto res = feed.next<unordered_map<string, float>>();

    REQUIRE_NOTHROW(res["bitfinex:/USD-BTC"] ==  7000);
    REQUIRE(res["bitfinex:/USD-BTC"] ==  7000);
    REQUIRE_NOTHROW(res["bitfinex:/USD-ETH"] ==  200);
    REQUIRE(res["bitfinex:/USD-ETH"] ==  200);
}

TEST_CASE("Internal py_wrapper feed")
{
    auto btc_price = source<float>({7000, 7500, 8300}); btc_price->rename("USD-BTC");
    auto eth_price = source<float>({200, 212, 400}); eth_price->rename("USD-ETH");

    auto btc_price2 = source<float>({7005, 7600, 8200}); btc_price2->rename("USD-BTC");
    auto eth_price2 = source<float>({201, 208, 402}); eth_price2->rename("USD-ETH");
    auto ltc_price = source<float>({56, 52, 60}); ltc_price->rename("USD-LTC");

    auto ex1 = Exchange("bitfinex", Service{}, {}, {btc_price, eth_price});
    auto ex2 = Exchange("binance", Service{}, {}, {btc_price2, eth_price2, ltc_price});

    auto wallets = {Wallet("bitfinex", 10000 * USD),
                    Wallet("bitfinex", 10 * BTC),
                    Wallet("bitfinex", 5 * ETH),
                    Wallet("binance", 1000 * USD),
                    Wallet("binance", 5 * BTC),
                    Wallet("binance", 20 * ETH),
                    Wallet("binance", 3 * LTC)};

    vector<string> wallet_ids;

    for(auto const& x : wallets)
    {
        Ledger::instance()->registerWallet(x);
        wallet_ids.push_back(x.ID());
    }
    Ledger::instance()->registerExchange(ex1);
    Ledger::instance()->registerExchange(ex2);

    Portfolio portfolio(USD, wallet_ids, nullptr);
    DataFeed feed(create_internal_streams(&portfolio), false);

    auto bitfinex_net_worth = 10000 + (10 * 7000) + (5 * 200);
    auto binance_net_worth = 1000 + (5 * 7005) + (20 * 201) + (3 * 56);

    std::unordered_map<string, float> data = {
            {"bitfinex:/USD-BTC", 7000},
            {"bitfinex:/USD-ETH", 200},
            {"bitfinex:/USD:/free", 10000},
            {"bitfinex:/USD:/locked", 0},
            {"bitfinex:/USD:/total", 10000},
            {"bitfinex:/BTC:/free", 10},
            {"bitfinex:/BTC:/locked", 0},
            {"bitfinex:/BTC:/total", 10},
            {"bitfinex:/BTC:/worth", 7000 * 10},
            {"bitfinex:/ETH:/free", 5},
            {"bitfinex:/ETH:/locked", 0},
            {"bitfinex:/ETH:/total", 5},
            {"bitfinex:/ETH:/worth", 200 * 5},
            {"binance:/USD-BTC", 7005},
            {"binance:/USD-ETH", 201},
            {"binance:/USD-LTC", 56},
            {"binance:/USD:/free", 1000},
            {"binance:/USD:/locked", 0},
            {"binance:/USD:/total", 1000},
            {"binance:/BTC:/free", 5},
            {"binance:/BTC:/locked", 0},
            {"binance:/BTC:/total", 5},
            {"binance:/BTC:/worth", 7005 * 5},
            {"binance:/ETH:/free", 20},
            {"binance:/ETH:/locked", 0},
            {"binance:/ETH:/total", 20},
            {"binance:/ETH:/worth", 201 * 20},
            {"binance:/LTC:/free", 3},
            {"binance:/LTC:/locked", 0},
            {"binance:/LTC:/total", 3},
            {"binance:/LTC:/worth", 56 * 3}
    };

    auto networth = 0.f;

    for(auto& item : data)
        networth += item.first.ends_with("worth") or item.first.ends_with("USD:/total") ? data[item.first] : 0.f;

    REQUIRE(networth == binance_net_worth + bitfinex_net_worth);

    auto next_res = feed.next<unordered_map<string, float>>();
    for(auto & item : data)
        REQUIRE(item.second == next_res[item.first]);

}

TEST_CASE("Exchange with wallets feed")
{
    auto btc_price = source<float>({7000, 7500, 8300}); btc_price->rename("USD-BTC");
    auto eth_price = source<float>({200, 212, 400}); eth_price->rename("USD-ETH");

    auto btc_price2 = source<float>({7005, 7600, 8200}); btc_price2->rename("USD-BTC");
    auto eth_price2 = source<float>({201, 208, 402}); eth_price2->rename("USD-ETH");
    auto ltc_price = source<float>({56, 52, 60}); ltc_price->rename("USD-LTC");

    auto ex1 = Exchange("bitfinex", Service{}, {}, {btc_price, eth_price});
    auto ex2 = Exchange("binance", Service{}, {}, {btc_price2, eth_price2, ltc_price});

    auto wallet_btc = Wallet(ex1.ID(), 10 * BTC);
    auto wallet_usd = Wallet(ex2.ID(), 1000 * USD);

    Ledger::instance()->registerExchange(ex1);
    Ledger::instance()->registerExchange(ex2);
    Ledger::instance()->registerWallet(wallet_usd);
    Ledger::instance()->registerWallet(wallet_btc);

    auto wallet_btc_ds = create_wallet_source(wallet_btc);
    auto wallet_usd_ds = create_wallet_source(wallet_usd, false);



//    auto streams = ex1.streams() + ex2.streams() + wallet_btc_ds + wallet_btc_ds;
//    feed = DataFeed(streams);


}