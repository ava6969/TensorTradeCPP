//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_EXCHANGE_PAIR_H
#define TENSORTRADECPP_EXCHANGE_PAIR_H

#include <string>
#include <utility>
#include <memory>
#include "tensortrade/core/types.h"
#include "trading_pair.h"
#include "tensortrade/oms/wallets/ledger.h"
#include "tensortrade/core/dexceptions.h"

namespace ttc
{
    class ExchangePair {

        string m_exchange_id, m_exchange_name;
        TradingPair m_trading_pair;

    public:
        ExchangePair()=default;
        ExchangePair(ExchangePair const&)=default;
        ExchangePair& operator=(ExchangePair const&)=default;

        explicit ExchangePair( const string &_exchange_name, const string &_exchange_id , TradingPair const& _pair):
                m_exchange_name(_exchange_name),
        m_exchange_id(_exchange_id),
                                                                                         m_trading_pair(_pair)
        {}

        inline TradingPair pair() const { return m_trading_pair; }

        [[nodiscard]] double price() const {  return Ctx::ctx()->exchanges(m_exchange_id).
        quote_price(m_trading_pair); }

        [[nodiscard]] auto exchange_id() const { return m_exchange_id; }
        [[nodiscard]] auto exchange_name() const { return m_exchange_name; }

        [[nodiscard]] inline double inverse_price() const { return price(); }

        bool operator==(ExchangePair const& other) const
        {
            return str() == other.str();
        }

        [[nodiscard]] inline string str() const
        {
            std::stringstream ss;
            ss << m_exchange_name << ":" << m_trading_pair.str();
            return ss.str();
        }

    };



}

#endif //TENSORTRADECPP_EXCHANGE_PAIR_H
