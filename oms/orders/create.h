//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_CREATE_H
#define TENSORTRADECPP_CREATE_H

#include "trade.h"
#include "wallets/portfolio.h"
#include "instruments/quantity.h"
#include "orders/order.h"
#include "wallets/wallet.h"
#include "criteria.h"

namespace ttc
{
    [[maybe_unused]] inline static std::unique_ptr<Order> marketOrder(TradeSide trade_side,
                                   ExchangePair const& exchangePair,
                                   float price, float size,
                                   Portfolio& portfolio)
    {
        auto _instrument = instrument(trade_side, exchangePair.pair());
        int _step = portfolio._clock.Step();
        return std::make_unique<Order>(_step,
                     trade_side,
                     TradeType::MARKET,
                     exchangePair,
                     Quantity{_instrument * size},
                     &portfolio,
                     price);
    }

    inline static std::unique_ptr<Order>  limitOrder(TradeSide trade_side,
                                                     ExchangePair const& exchangePair,
                                   float limit_price, float size,
                                   Portfolio& portfolio,
                                   int start=-1, int end=-1)
    {
        auto _instrument = instrument(trade_side, exchangePair.pair());
        int _step = portfolio._clock.Step();
        return std::make_unique<Order>(_step,
                     trade_side,
                     TradeType::LIMIT,
                     exchangePair,
                     Quantity{_instrument * size},
                     &portfolio,
                     limit_price, "", start, end);
    }

    inline static std::shared_ptr<Order>  hiddenLimitOrder(TradeSide trade_side,
                                                     ExchangePair const& exchangePair,
                                                     float limit_price, float size,
                                                     Portfolio& portfolio,
                                                     int start=-1, int end=-1)
    {
        auto _instrument = instrument(trade_side, exchangePair.pair());
        int _step = portfolio._clock.Step();
        return std::make_shared<Order>(_step,
                                       trade_side,
                                       TradeType::MARKET,
                                       exchangePair,
                                       Quantity{_instrument * size},
                                       &portfolio,
                                       limit_price, "", start, end,
                                       std::make_unique<Limit>(limit_price));
    }

    inline static std::shared_ptr<Order> riskManagedOrder(TradeSide trade_side, TradeType tradeType,
                                   ExchangePair const& exchangePair, float price, Quantity<float> quantity,
                                   float down_percent, float up_percent, Portfolio* portfolio,
                                   std::optional<int> start=std::nullopt,
                                   std::optional<int> end=std::nullopt)
    {
        auto _instrument = instrument(trade_side, exchangePair.pair());

        std::shared_ptr<Order> order =  std::make_shared<Order>(portfolio->_clock.Step(),
                trade_side,
                tradeType,
                exchangePair,
                quantity,
                portfolio,
                price, "", start, end);

        Stop* left = new Stop(Stop::down, down_percent);
        Stop* right = new Stop(Stop::up, up_percent);
        auto res =  *left ^ right;

        auto cr = std::make_shared<CriteriaBinOp>(std::move(res));
        cr->right = new Stop(Stop::up, up_percent);
        cr->left = new Stop(Stop::down, down_percent);

        OrderSpec risk_management( trade_side == TradeSide::BUY ? TradeSide::SELL : TradeSide::BUY,
                                          TradeType::MARKET, exchangePair,
                                          std::move(cr)
                                        );

        order->addOrderSpec(risk_management);
        return std::move(order);

    }

    inline static std::unique_ptr<Order>  proportionOrder( Portfolio* portfolio,
                                                           Wallet const& src,
                                                           Wallet const& tgt,
                                                           float proportion)
    {
        assert(0.0 < proportion <= 1.0);

        auto is_src_base = src.instrument() == portfolio->baseInstrument();
        auto is_tgt_base = tgt.instrument() == portfolio->baseInstrument();

        TradingPair pair;
        if(is_src_base or is_tgt_base)
        {
            if(is_src_base)
                pair = src.instrument() / tgt.instrument();
            else
                pair = tgt.instrument() / src.instrument();

            ExchangePair exchange_pair(src.exchange(), pair);
            auto balance = src.Balance().asFloat();

            auto size = std::min<float>(balance * proportion, balance);
            auto qty = (size * src.instrument()).quantize();

            return std::make_unique<Order>(portfolio->_clock.Step(),
                                           is_src_base ? TradeSide::BUY : TradeSide::SELL,
                                           TradeType::MARKET,
                                           exchange_pair,
                                           qty,
                                           portfolio,
                                           exchange_pair.price(),
                                           "",
                                           portfolio->_clock.Step(),
                                           portfolio->_clock.Step() + 1);
        }

        pair = portfolio->baseInstrument() / src.instrument();
        ExchangePair exchange_pair(src.exchange(), pair);

        auto balance = src.Balance().asFloat();
        auto size = std::min<float>(balance * proportion, balance);
        auto qty = (size * src.instrument()).quantize();

        auto order = std::make_unique<Order>(portfolio->_clock.Step(),
                                             TradeSide::SELL,
                                             TradeType::MARKET,
                                             exchange_pair,
                                             qty,
                                             portfolio,
                                             exchange_pair.price(),
                                             "",
                                             portfolio->_clock.Step(),
                                             portfolio->_clock.Step() + 1);

        auto _pair = portfolio->baseInstrument() / tgt.instrument();

        OrderSpec spec(TradeSide::BUY, TradeType::MARKET, exchange_pair, {});

        order->addOrderSpec(std::move(spec));
        return std::move(order);
    }

}

#endif //TENSORTRADECPP_CREATE_H
