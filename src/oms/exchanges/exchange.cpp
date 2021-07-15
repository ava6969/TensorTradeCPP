//
// Created by dewe on 6/30/21.
//

#include "tensortrade/oms/exchanges/exchange.h"

#include <utility>
#include "tensortrade/oms/orders/order.h"
#include "tensortrade/oms/wallets/wallet.h"
#include "tensortrade/oms/wallets/portfolio.h"
#include "tensortrade/oms/instruments/trading_pair.h"
#include "tensortrade/oms/services/execution/simulated.h"
#include "tensortrade/oms/orders/trade.h"

namespace ttc
{

    Exchange::Exchange(std::string _name,
                       ttc::Service const& callable,
                       const ttc::ExchangeOptions &options,
                       vector<Float64Stream> const& _streams): name(std::move(_name)), service(callable),
                                                               options(options)  {
        using namespace std::string_literals;
        size_t start_idx{0};
        string head{""};
        if (not Named::namespaces.empty())
        {
            head = Named::namespaces.top();
            start_idx += head.size() + 2;
        }

        for(auto* s : _streams)
        {
            string pair_;
            auto d_name = s->Name().substr(start_idx);
            for(auto const& c : d_name)
            {
                pair_ += isalnum(c) ? c : '/' ;
            }
            auto new_name = head + ":/" + name + std::string(":/") + d_name;
            price_streams[pair_] = s->rename(new_name);
        }

    }

    vector<Float64Stream> Exchange::streams() const{

        vector<Float64Stream> casts(price_streams.size());
        std::transform(price_streams.begin(), price_streams.end(), casts.begin(), [](auto const& x) {
            return x.second;
        });
        return casts;
    }

    bool Exchange::isPairTradable(TradingPair const& trading_pair) const {
        return price_streams.contains(trading_pair.str());
    }

    void Exchange::executeOrder(Order& order, Portfolio& portfolio) const{

        auto _trade = service(order,
                             portfolio.getWallet(id, order.pair().base()),
                             portfolio.getWallet(id, order.pair().quote()),
                             quote_price(order.pair()),
                             options,
                             _clock);

        if(_trade)
            order.fill(_trade.value());

    }

    float Exchange::quote_price(TradingPair const& trading_pair) const  {

        auto price = price_streams.at(trading_pair.str())->value();
        if(price == 0.f)
        {
            throw std::runtime_error(string("Price of trading pair ").
                    append(trading_pair.str()).
                    append(" is 0. Please check your input py_wrapper to make sure there always is "
                           "a valid (nonzero) price."));
        }

        return price;
    }

    Exchange &Exchange::exchange(const string &exchange_id) {
        return Ctx::ctx()->exchanges(exchange_id);
    }

    Exchange& Exchange::exchange(class ExchangePair const& ex)
    {
        return Ctx::ctx()->exchanges(ex.exchange_id());
    }


}


