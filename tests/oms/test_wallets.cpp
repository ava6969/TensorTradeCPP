//
// Created by dewe on 7/8/21.
//

#include "catch.hpp"
#include <include/oms/exchanges/exchange.h>
#include <include/oms/orders/order_spec.h>
#include "include/oms/instruments/exchange_pair.h"
#include "include/oms/instruments/trading_pair.h"
#include "include/core/dexceptions.h"
#include "include/oms/wallets/wallet.h"


using namespace ttc;

auto _wallet_usd(string exchange_id)
{
    return Wallet(exchange_id, 10000.f * USD);
}

auto _wallet_btc(string exchange_id)
{
    return Wallet(exchange_id, 1.f * BTC);
}

auto _wallet_eth(string exchange_id)
{
    return Wallet(exchange_id, 10.f * ETH);
}

auto _wallet_xrp(string exchange_id)
{
    return Wallet(exchange_id, 5000.f * XRP);
}


void init_helper(Portfolio const& p,
                        Instrument base_inst,
                        Quantity<float> base_bal,
                        Quantity<float> init_bal,
                        int wallet_sz)
{
    auto res = p.baseInstrument() == base_inst;
    auto res1 = p.baseBalance() == base_bal;
    auto res2 = p.initialBalance() == init_bal;
    auto res3 = p.wallets().size() == wallet_sz;

    REQUIRE(res);
    REQUIRE(res1);
    REQUIRE(res3);
    REQUIRE(res2);
}

Portfolio portfolio_locked(vector<std::string>& wallets)
{
    string const BITSTAMP = "bitstamp";

    auto wallet_usd = _wallet_usd(BITSTAMP);
    auto wallet_btc = _wallet_btc(BITSTAMP);
    auto wallet_eth = _wallet_eth(BITSTAMP);
    auto wallet_xrp = _wallet_xrp(BITSTAMP);

    wallets = vector<string>{wallet_btc.ID(), wallet_eth.ID(), wallet_usd.ID(), wallet_xrp.ID()};

    Ledger::instance()->registerWallet(move(wallet_usd));
    Ledger::instance()->registerWallet(move(wallet_btc));
    Ledger::instance()->registerWallet(move(wallet_eth));
    Ledger::instance()->registerWallet(move(wallet_xrp));

    Portfolio portfolio(USD, wallets, nullptr);
    portfolio.clock(Ledger::instance()->exchanges("bitstamp")._clock);

    auto allocate = [](Wallet& wallet, Quantity<float> amount, string const& id) -> Wallet
    {
        auto _amt = wallet.withdraw(amount, "test");
        auto _amt2 = wallet.deposit(amount.lock_for(id), "test");
        return wallet;
    };

    wallet_usd = allocate(Ledger::instance()->wallet(wallets[2]), 50.f * USD, "1");
    wallet_usd = allocate(Ledger::instance()->wallet(wallets[2]), 100.f * USD, "2");
    wallet_btc = allocate(Ledger::instance()->wallet(wallets[0]), 0.5f * BTC, "3");
    wallet_btc = allocate(Ledger::instance()->wallet(wallets[0]), 0.25f * BTC, "4");
    wallet_eth = allocate(Ledger::instance()->wallet(wallets[1]), 5.f * ETH, "5");
    wallet_eth = allocate(Ledger::instance()->wallet(wallets[1]), 2.f * ETH, "6");
    wallet_xrp = allocate(Ledger::instance()->wallet(wallets[3]), 250.f * XRP, "7");
    wallet_xrp = allocate(Ledger::instance()->wallet(wallets[3]), 12.f * XRP, "8");

    return std::move(portfolio);
}


TEST_CASE("Portfolio")
{
    Service service;

    Ledger::instance()->registerExchange(Exchange{"bitstamp", service, {},
                                                  vector<FloatStream>{source<float>({20, 124.33, 1337.2})}});

    Clock clk;
    Ledger::instance()->exchanges("bitstamp")._clock = &clk;

    auto  const ZERO_USD = 0.f * USD;
    auto  const ZERO_BTC = 0.f * BTC;
    std::vector<std::string> wallets;
    auto  portfolio = portfolio_locked(wallets);

    SECTION("test_init_empty")
    {
        Portfolio portfolio(USD, {}, nullptr);
        init_helper(portfolio, USD, ZERO_USD, ZERO_USD, 0);
    }

    SECTION("test init from wallets")
    {
        auto& exchange = Ledger::instance()->exchanges("bitstamp");
        auto& fx_exchange = Ledger::instance()->exchanges("bitstamp");
        auto& binance_exchange = Ledger::instance()->exchanges("bitstamp");

        Wallet w1{"bitstamp", 10000.f * USD};
        Wallet w2{"bitstamp", ZERO_BTC};

        auto wallets = std::vector<string>{w1.ID(), w2.ID()};

        Ledger::instance()->registerWallet(move(w1));
        Ledger::instance()->registerWallet(move(w2));

        Portfolio portfolio(USD, wallets, nullptr);

        init_helper(portfolio, USD, 10000.f * USD, 10000.f * USD, 2);

    }

    SECTION("test balance")
    {

        auto expr1 = portfolio.balance(USD) == 9850.f;
        auto expr2 = portfolio.balance(BTC) == 0.25f;
        auto expr3 = portfolio.balance(ETH) == 3.f;
        auto expr4 = portfolio.balance(XRP) == 4738.f;

        REQUIRE(expr1);
        REQUIRE(expr2);
        REQUIRE(expr3);
        REQUIRE(expr4);
    }

    SECTION("test locked_balance")
    {

        auto expr1 = portfolio.lockedBalance(USD) == 150.f;
        auto expr2 = portfolio.lockedBalance(BTC) == 0.75f;
        auto expr3 = portfolio.lockedBalance(ETH) == 7.f;
        auto expr4 = portfolio.lockedBalance(XRP) ==  262.f;

        REQUIRE(expr1);
        REQUIRE(expr2);
        REQUIRE(expr3);
        REQUIRE(expr4);
    }

    SECTION("test total_balance")
    {

        auto expr1 = portfolio.totalBalance(USD) == 10000.f;
        auto expr2 = portfolio.totalBalance(BTC) == 1.f;
        auto expr3 = portfolio.totalBalance(ETH) == 10.f;
        auto expr4 = portfolio.totalBalance(XRP) == 5000.f;

        REQUIRE(expr1);
        REQUIRE(expr2);
        REQUIRE(expr3);
        REQUIRE(expr4);
    }

    SECTION("test balances")
    {

        vector<Quantity<float>> balances        { 0.25f * BTC, 3.f * ETH, 9850.f * USD, 4738.f * XRP};
        vector<Quantity<float>> locked_balances {  0.75f * BTC, 7.f * ETH, 150.f * USD, 262.f * XRP};
        vector<Quantity<float>> total_balances  {  1.f * BTC, 10.f * ETH, 10000.f * USD, 5000.f * XRP};

        auto lb = portfolio.lockedBalances();
        auto expr1 = std::equal(lb.begin(), lb.end() ,
                                locked_balances.begin(),
                                locked_balances.end());

        auto b = portfolio.balances();
        auto expr2 = std::equal(b.begin(), b.end() ,
                                balances.begin(),
                                balances.end());

        auto tb = portfolio.totalBalances();
        auto expr3 = std::equal(tb.begin(), tb.end() ,
                                total_balances.begin(),
                                total_balances.end());

        REQUIRE(expr1);
        REQUIRE(expr2);
        REQUIRE(expr3);
    }


}

TEST_CASE("Wallet manipulation")
{
    Service service;

    auto usd_btc = source<float>({7117.00, 6750.00});
    usd_btc->rename("USD-BTC");

    auto usd_eth = source<float>({143.f, 135.00});
    usd_eth->rename("USD-ETH");

    auto usd_xrp = source<float>({0.22f, 0.30});
    usd_xrp->rename("USD-XRP");

    Ledger::instance()->registerExchange(Exchange{"bitstamp", service, {}, {move(usd_btc),
                                                                            move(usd_eth),
                                                                            move(usd_xrp)}});

    Clock clk;
    Ledger::instance()->exchanges("bitstamp")._clock = &clk;

    auto  const ZERO_USD = 0.f * USD;
    auto  const ZERO_BTC = 0.f * BTC;

    std::vector<std::string> wallets;
    auto  portfolio = portfolio_locked(wallets);

    auto& w = Ledger::instance()->wallet(wallets.back());
    auto & w2 = portfolio.getWallet(Ledger::instance()->exchanges("bitstamp").ID(),
                                    XRP);
    REQUIRE(w == w2);

    Wallet bch{"bitstamp", 1000.f * BCH};
    Ledger::instance()->registerWallet(bch);
    portfolio.add(bch.ID());

    auto p_wallets = portfolio.wallets();
    REQUIRE(std::find(p_wallets.begin(), p_wallets.end(), bch.ID()) != p_wallets.end());

    portfolio.remove(bch.ID());
    p_wallets = portfolio.wallets();
    REQUIRE(std::find(p_wallets.begin(), p_wallets.end(), bch.ID()) == p_wallets.end());
}

TEST_CASE("Test transfer")
{

}