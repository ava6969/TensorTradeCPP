//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_CREATE_H
#define TENSORTRADECPP_CREATE_H

#include "trade.h"
#include "tensortrade/oms/wallets/portfolio.h"
#include "tensortrade/oms/instruments/quantity.h"
#include "order.h"
#include "tensortrade/oms/wallets/wallet.h"
#include "criteria.h"

namespace ttc
{
    [[maybe_unused]] inline static std::unique_ptr<Order> marketOrder(TradeSide trade_side,
                                   ExchangePair const& exchangePair,
                                   float price, float size,
                                   Portfolio& portfolio)
    {
        auto _instrument = instrument(trade_side, exchangePair.pair());
        int _step = portfolio._clock->Step();
        return std::make_unique<Order>(_step,
                     trade_side,
                     TradeType::MARKET,
                     exchangePair,
                     Quantity{_instrument * size},
                     portfolio,
                     price);
    }

    inline static std::optional<Order>  limitOrder(TradeSide trade_side,
                                                     ExchangePair const& exchangePair,
                                   float limit_price, float size,
                                   Portfolio& portfolio,
                                   int start=-1, int end=-1)
    {
        auto _instrument = instrument(trade_side, exchangePair.pair());
        int _step = portfolio._clock->Step();
        return Order{_step,
                     trade_side,
                     TradeType::LIMIT,
                     exchangePair,
                     Quantity{_instrument * size},
                     portfolio,
                     limit_price, "", start, end};
    }

    inline static std::unique_ptr<Order>  hiddenLimitOrder(TradeSide trade_side,
                                                     ExchangePair const& exchangePair,
                                                     float limit_price, float size,
                                                     Portfolio& portfolio,
                                                     int start=-1, int end=-1)
    {
        auto _instrument = instrument(trade_side, exchangePair.pair());
        int _step = portfolio._clock->Step();

        auto lim =  [limit_price](Order const& o, Exchange const& ex) -> bool
        {
             Limit l(limit_price);

             return l(o, ex);
        };
        return std::make_unique<Order>(_step,
                                       trade_side,
                                       TradeType::MARKET,
                                       exchangePair,
                                       Quantity{_instrument * size},
                                       portfolio,
                                       limit_price, "", start, end,
                                       std::move(lim));
    }

    inline static std::optional<Order> riskManagedOrder(TradeSide trade_side, TradeType tradeType,
                                   ExchangePair const& exchangePair, double price, Quantity quantity,
                                   float down_percent, float up_percent, Portfolio const& portfolio,
                                   std::optional<int> start=std::nullopt,
                                   std::optional<int> end=std::nullopt)
    {
        auto _instrument = instrument(trade_side, exchangePair.pair());

        std::optional<Order> order =  Order{portfolio._clock->Step(),
                trade_side,
                tradeType,
                exchangePair,
                quantity,
                portfolio,
                price, "", start, end};

        auto cr =  [down_percent, up_percent](Order const& o, Exchange const& ex) -> bool
        {
            auto criteria = Stop(Stop::down, down_percent) ^ Stop(Stop::up, up_percent);
            return criteria(o, ex);
        };

        OrderSpec risk_management( trade_side == TradeSide::BUY ? TradeSide::SELL : TradeSide::BUY,
                                          TradeType::MARKET, exchangePair,
                                          std::move(cr));

        order->addOrderSpec(std::move(risk_management));
        return std::move(order);

    }

    inline static std::optional<Order> proportionOrder( Portfolio const& portfolio,
                                                           Wallet const& src,
                                                           Wallet const& tgt,
                                                           float proportion)
    {
        assert(0.0 < proportion <= 1.0);

        auto is_src_base = src.instrument() == portfolio.baseInstrument();
        auto is_tgt_base = tgt.instrument() == portfolio.baseInstrument();

        TradingPair pair;
        if(is_src_base or is_tgt_base)
        {
            if(is_src_base)
                pair = src.instrument() / tgt.instrument();
            else
                pair = tgt.instrument() / src.instrument();

            ExchangePair exchange_pair(src.exchange().Name(), src.exchange_id(), pair);
            auto balance = src.Balance().asDouble();

            auto size = std::min<double>(balance * proportion, balance);
            auto qty = (size * src.instrument()).quantize();

            return Order{portfolio._clock->Step(),
                                           is_src_base ? TradeSide::BUY : TradeSide::SELL,
                                           TradeType::MARKET,
                                           exchange_pair,
                                           qty,
                                           portfolio,
                                           exchange_pair.price(),
                                           "",
                                           portfolio._clock->Step(),
                                           portfolio._clock->Step() + 1};
        }

        pair = portfolio.baseInstrument() / src.instrument();
        ExchangePair exchange_pair(src.exchange().Name(), src.exchange_id(), pair);

        auto balance = src.Balance().asDouble();
        auto size = std::min<double>(balance * proportion, balance);
        auto qty = (size * src.instrument()).quantize();

        std::optional<Order> order = Order{portfolio._clock->Step(),
                                             TradeSide::SELL,
                                             TradeType::MARKET,
                                             exchange_pair,
                                             qty,
                                             portfolio,
                                             exchange_pair.price(),
                                             "",
                                             portfolio._clock->Step(),
                                             portfolio._clock->Step() + 1};

        auto _pair = portfolio.baseInstrument() / tgt.instrument();

        OrderSpec spec(TradeSide::BUY, TradeType::MARKET, exchange_pair, {});

        order->addOrderSpec(std::move(spec));
        return std::move(order);
    }

}

#endif //TENSORTRADECPP_CREATE_H
