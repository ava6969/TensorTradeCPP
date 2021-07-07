//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_EXCHANGE_PAIR_H
#define TENSORTRADECPP_EXCHANGE_PAIR_H

#include <string>
#include <utility>
#include <memory>
#include "../../core/types.h"
#include "trading_pair.h"

namespace ttc
{
    class ExchangePair {

        class Exchange* m_exchange;
        TradingPair m_trading_pair;

    public:
        ExchangePair()=default;
        explicit ExchangePair( Exchange* _exchange, TradingPair const& _pair);

        [[nodiscard]] float price() const;
        [[nodiscard]] Exchange*  exchange() const;
        [[nodiscard]] TradingPair const& pair() const;

        [[nodiscard]] double inverse_price() const;

        bool operator==(ExchangePair const& other) const
        {
            return str() == other.str();
        }

        [[nodiscard]] string str() const;

    };


}

#endif //TENSORTRADECPP_EXCHANGE_PAIR_H
