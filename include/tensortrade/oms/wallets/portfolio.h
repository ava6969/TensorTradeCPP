//
// Created by dewe on 7/1/21.
//

#ifndef TENSORTRADECPP_PORTFOLIO_H
#define TENSORTRADECPP_PORTFOLIO_H

#include "tensortrade/core/base.h"
#include "tensortrade/oms/orders/order_listener.h"
#include "tensortrade/oms/instruments/quantity.h"
#include "string"
#include "unordered_map"
#include "vector"
#include "functional"
#include "map"
#include "torch/torch.h"

namespace ttc
{

    using std::string;
    using Quantities = std::vector<Quantity>;
    using PerformanceListener = std::function<void(std::map<int, std::unordered_map<string, double> >)>;

    class Portfolio :
        public TimedIdentifiable{

        Instrument base_instrument;
        std::map<std::pair<string, string>, string> m_wallet_ids;

        Quantity initial_balance{};
        double initial_net_worth{};
        double net_worth{};

        std::map<int, std::unordered_map<string, double> > m_performance;
        OrderListener* order_listener{};
        std::optional<PerformanceListener>  performance_listener;
        std::vector<string> keys;

    public:
        Portfolio(Instrument const& instrument,
                  std::vector<string> const& wallets,
                  OrderListener* order_listener,
                  std::optional<PerformanceListener> const& performance_listener = std::nullopt);
        Portfolio()=default;
        [[nodiscard]] std::vector<string> wallets() const;

        [[nodiscard]] std::vector<std::string> exchanges();

        [[nodiscard]] inline auto initialBalance() const { return initial_balance; }
        [[nodiscard]] inline auto baseBalance() const { return balance(base_instrument); }
        [[nodiscard]] inline auto baseInstrument() const { return base_instrument; }
        [[nodiscard]] inline auto initialNetWorth() const { return initial_net_worth; }
        [[nodiscard]] inline auto netWorth() const { return net_worth; }
        [[nodiscard]] inline auto profitLoss() const { return 1.f - net_worth / initial_net_worth; }
        [[nodiscard]] inline auto& performance() const { return m_performance;}
        [[nodiscard]] inline auto orderListener() const { return order_listener;}

        [[nodiscard]] std::vector<ExchangePair> exchangePairs() const;

        [[nodiscard]] Quantities balances() const;
        [[nodiscard]] std::vector<Quantity>  lockedBalances() const;
        [[nodiscard]] Quantities totalBalances() const;
        [[nodiscard]] Quantity balance(Instrument const& instrument) const;
        [[nodiscard]] Quantity lockedBalance(Instrument const& instrument) const;
        [[nodiscard]] Quantity totalBalance(Instrument const& instrument) const;

        [[nodiscard]] class Wallet& getWallet(string const& exchangeID, Instrument const& instrument);

        void add(string const& wallet);
        void remove(string const& wallet);

        std::vector<string> removePair(std::string  const& exchange, Instrument const& instrument);
        std::vector<string> findKeys(const std::unordered_map<string, double>&);

        void reset();

        void onNext(std::unordered_map<string, std::unordered_map<string, double>> const& data);
    };
}



#endif //TENSORTRADECPP_PORTFOLIO_H
