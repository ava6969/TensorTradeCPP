//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_TRADE_H
#define TENSORTRADECPP_TRADE_H

#include <any>
#include "tensortrade/feed/core/base.h"
#include "tensortrade/oms/instruments/trading_pair.h"
#include "tensortrade/oms/instruments/instrument.h"
#include "tensortrade/oms/instruments/exchange_pair.h"
#include "tensortrade/oms/instruments/quantity.h"
#include "tensortrade/core/types.h"

namespace ttc
{

    static auto instrument(TradeSide const& side, TradingPair const& pair)
    {
        return side == TradeSide::BUY ? pair.base() : pair.quote();
    }

    struct TradeOption
    {
        string order_id;
        int step;
        ExchangePair exchange_pair;
        TradeSide side;
        TradeType type;
        Quantity quantity;
        double price;
        Quantity commission;
    };


    class Trade : public TimedIdentifiable
    {

        TradeOption opt;

    public:
        Trade(const TradeOption& option): opt(option) {}
        Trade(TradeOption&& option): opt(std::move(option)) {}
        Trade() = default;
        Trade(Trade const&)=default;
        Trade& operator=(Trade const&)=default;

        inline auto baseInstrument() const { return opt.exchange_pair.pair().base();}
        inline auto quoteInstrument() const { return opt.exchange_pair.pair().quote();}
        inline auto size() const { return opt.quantity.size;}

        inline auto price() const { return opt.price;}
        inline auto price(float _price) { opt.price = _price; }

        inline auto commission() const { return opt.commission;}
        inline auto commission(Quantity const& _commission) { opt.commission = _commission; }

        inline auto isBuy() const { return opt.side == TradeSide::BUY; }
        inline auto isSell() const { return opt.side == TradeSide::SELL; }

        inline auto isLimitOrder() const { return opt.type == TradeType::LIMIT; }
        inline auto isMarketOrder() const { return opt.type == TradeType::MARKET;}

        inline auto orderID() const { return opt.order_id; }
        inline auto quantity() const { return opt.quantity; }

        [[nodiscard]] inline unordered_map<string, std::string> to_dict() const
        {
            return {{"id", id},
                    {"order_id", opt.order_id}};
        }

        inline unordered_map<string, string> to_json() const
        {
            return {{"id",          id},
                    {"order_id",    opt.order_id}};
        }

        [[nodiscard]] string str() const
        {
            std::stringstream ss;
            ss << "Trade: ";
            for(auto const&[k, v] : to_dict())
            {
                ss << k << "=" << any_cast<string>(v) << ", ";
            }
            return ss.str();
        }

    };

}


#endif //TENSORTRADECPP_TRADE_H
