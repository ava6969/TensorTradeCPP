//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_TRADING_PAIR_H
#define TENSORTRADECPP_TRADING_PAIR_H

#include <optional>
#include "string"
#include "instrument.h"

namespace ttc{

    using std::string;

    class TradingPair
    {
        Instrument m_base;
        Instrument m_quote;

    public:
        TradingPair()=default;
        TradingPair(TradingPair const&)=default;
        TradingPair& operator=( TradingPair const&)=default;
        TradingPair( const Instrument& _base, const Instrument& _quote);

        [[nodiscard]] size_t hash() const;
        [[nodiscard]] string str() const;

        bool operator==(TradingPair const& other) const;
        bool operator!=(TradingPair const& other) const;

        [[nodiscard]] Instrument base() const { return m_base;}
        [[nodiscard]] Instrument quote() const { return m_quote;}
    };
}



#endif //TENSORTRADECPP_TRADING_PAIR_H
