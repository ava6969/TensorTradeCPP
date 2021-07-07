//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_EXCHANGE_H
#define TENSORTRADECPP_EXCHANGE_H

#include <services/execution/simulated.h>
#include "core/base.h"
#include "variant"
#include "functional"
#include "unordered_map"
#include "../../core/types.h"

namespace ttc
{

    struct ExchangeOptions {

        bool is_live{false};
        float commission{0.003}, min_trade_size{1e-6}, max_trade_size{1e6},
        min_trade_price{1e-8}, max_trade_price{1e8};

    };

    class Exchange : public TimedIdentifiable {

        string name;
        Service service;
        ExchangeOptions options;
        std::unordered_map<string, FloatStream > price_streams{};

    public:
        Exchange()=default;
        Exchange(Exchange const&)=default;
        Exchange& operator=(Exchange const&)=default;
        explicit Exchange(string _name):name(move(_name)){}
        Exchange(string _name,
                 Service const& callable,
                 ExchangeOptions const& options,
                 vector<FloatStream> const& _streams);

        vector<FloatStream> streams();

        float quote_price(class TradingPair const& trading_pair) const;

        bool isPairTradable(TradingPair const& trading_pair) const;

        void executeOrder(class Order* order, class Portfolio* portfolio) const;

        [[nodiscard]] inline auto& Options() const { return options; }

        [[nodiscard]] inline auto Name() const& { return name; }


    };

    using Exchanges = vector<Exchange>;
}



#endif //TENSORTRADECPP_EXCHANGE_H
