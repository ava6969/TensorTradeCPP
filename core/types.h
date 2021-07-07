//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_TYPES_H
#define TENSORTRADECPP_TYPES_H

#include "string"

using std::string;

namespace ttc
{

    enum class TradeType
    {
        LIMIT,
        MARKET
    };

    enum class TradeSide
    {
        BUY,
        SELL
    };

    static string str(TradeType const& e)
    {
        return e == TradeType::LIMIT ? "limit" : "market";
    }

    static string str(TradeSide const& e)
    {
        return e == TradeSide::BUY ? "buy" : "sell";
    }


};

#endif //TENSORTRADECPP_TYPES_H
