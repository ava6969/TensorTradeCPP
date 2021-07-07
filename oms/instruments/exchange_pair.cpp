//
// Created by dewe on 7/3/21.
//

#include "exchange_pair.h"
#include "exchanges/exchange.h"
#include "trading_pair.h"
#include "instrument.h"

namespace ttc
{
    ExchangePair::ExchangePair(ttc::Exchange* _exchange, ttc::TradingPair const& _pair):m_exchange(_exchange),
    m_trading_pair(_pair)
    {}

    float ExchangePair::price() const
    {
        return m_exchange->quote_price(m_trading_pair);
    }

    double ExchangePair::inverse_price() const
    {
        auto quantization = std::pow(10, -m_trading_pair.quote.precision);
        return std::scalbln(std::pow(price(), -1), quantization);
    }

    string ExchangePair::str() const
    {
        std::stringstream ss;
        ss << m_exchange->Name() << ":" << m_trading_pair.str();
        return ss.str();
    }

    Exchange* ExchangePair::exchange() const {
        return m_exchange;
    }

    TradingPair const& ExchangePair::pair() const {
        return m_trading_pair;
    }


}
