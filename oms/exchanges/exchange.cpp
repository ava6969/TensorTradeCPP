//
// Created by dewe on 6/30/21.
//

#include "exchange.h"

#include <utility>
#include "orders/order.h"
#include "wallets/wallet.h"
#include "wallets/portfolio.h"
#include "instruments/trading_pair.h"
#include "services/execution/simulated.h"
#include "orders/trade.h"

namespace ttc
{

    Exchange::Exchange(std::string _name,
                       ttc::Service const& callable,
                       const ttc::ExchangeOptions &options,
                       vector<FloatStream> const& _streams):name(std::move(_name)), service(callable),
                       options(options)  {

        for(auto const& s : _streams)
        {
            string pair_;
            for(auto const& c : s->Name())
            {
                pair_ += isalnum(c) ? c : '/' ;
            }
            auto new_name = name + std::string(":/") + s->Name();
            price_streams[pair_] = s;
            price_streams[pair_]->rename<Stream<float>>(new_name);
        }

    }

    vector<FloatStream> Exchange::streams() {

        vector<FloatStream> casts(price_streams.size());
        std::transform(price_streams.begin(), price_streams.end(), casts.begin(), [](auto const& x) {
            return x.second;
        });
        return casts;
    }

    bool Exchange::isPairTradable(TradingPair const& trading_pair) const {
        return price_streams.contains(trading_pair.str());
    }

    void Exchange::executeOrder(Order* order, Portfolio* portfolio) const {

        auto _trade = service(order,
                             portfolio->getWallet(id, order->pair().base),
                             portfolio->getWallet(id, order->pair().quote),
                             quote_price(order->pair()),
                             options,
                             _clock);

        if(_trade)
            order->fill(move(_trade));

    }

    float Exchange::quote_price(TradingPair const& trading_pair) const  {

        auto price = std::get<float>(price_streams.at(trading_pair.str())->Value());
        if(price == 0.f)
        {
            throw std::runtime_error(string("Price of trading pair ").
                    append(trading_pair.str()).
                    append(" is 0. Please check your input data to make sure there always is "
                           "a valid (nonzero) price."));
        }

//        price = std::scalbln(price, std::pow(10, -trading_pair.base.precision));
//        if(price == 0.f)
//        {
//            throw std::runtime_error(string("Price quantized in base currency precision (").
//                    append(std::to_string(trading_pair.base.precision)).
//                    append(" would amount to 0 ").append(trading_pair.base.str()).
//                    append("Please consider defining a custom instrument with a higher precision."));
//        }
        return price;
    }


}


