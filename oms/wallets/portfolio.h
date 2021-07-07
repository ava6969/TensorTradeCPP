//
// Created by dewe on 7/1/21.
//

#ifndef TENSORTRADECPP_PORTFOLIO_H
#define TENSORTRADECPP_PORTFOLIO_H

#include "../../core/base.h"
#include "orders/order_listener.h"
#include "instruments/quantity.h"
#include "string"
#include "unordered_map"
#include "vector"
#include "functional"
#include "map"
#include "torch/torch.h"

namespace ttc
{

    using std::string;
    using Quantities = std::vector<Quantity<float>>;
    using WalletMap = std::map<std::pair<string, string>, class Wallet>;
    using Wallets = std::vector<Wallet>;

    using PerformanceListener = std::function<void(std::map<int, std::unordered_map<string, string> >)>;

    class Portfolio : public TimedIdentifiable {

        Instrument base_instrument;
        OrderListener* order_listener{};
        std::optional<PerformanceListener>  performance_listener;
        WalletMap _wallets;
        Quantity<float> initial_balance;
        float initial_net_worth{}, net_worth{};
        std::map<int, std::unordered_map<string, string> > m_performance;
        std::vector<string> keys;

    public:
        Portfolio(Instrument const& instrument,
                  Wallets const& wallets,
                  OrderListener* order_listener,
                  std::optional<PerformanceListener> const& performance_listener = std::nullopt);

        Wallets wallets() const;

        std::vector< class Exchange*> exchanges();
        static class Ledger ledger();

        inline auto initialBalance() const { return initial_balance; }
        inline auto baseBalance() const { return balance(base_instrument); }
        inline auto baseInstrument() const { return base_instrument; }
        inline auto initialNetWorth() const { return initial_net_worth; }
        inline auto netWorth() const { return net_worth; }
        inline auto profitLoss() const { return 1.f - net_worth / initial_net_worth; }
        inline auto performance() const { return m_performance;}
        inline auto orderListener() const { return order_listener;}

        std::vector<ExchangePair> exchangePairs();

        Quantities balances() const;
        std::vector<Quantity<int>>  lockedBalances() const;
        Quantities totalBalances() const;
        Quantity<float> balance(Instrument const& instrument) const;
        Quantity<float> lockedBalance(Instrument const& instrument) const;
        Quantity<float> totalBalance(Instrument const& instrument) const;

        Wallet* getWallet(string const& exchangeID, Instrument const& instrument);

        void add(Wallet const& wallet);
        void add(std::tuple<Exchange*, Instrument, int> const& wallet);
        void remove(Wallet const& wallet);
        void remove(std::tuple<Exchange*, Instrument, int> const& wallet);

        std::vector<string> removePair(Exchange const& exchange, Instrument const& instrument);
        std::vector<string> findKeys(std::unordered_map<string, float>);

        void reset();

        void onNext(std::unordered_map<string, std::unordered_map<string, float>> const& data);
    };
}



#endif //TENSORTRADECPP_PORTFOLIO_H
