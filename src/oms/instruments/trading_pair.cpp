//
// Created by dewe on 6/30/21.
//

#include "tensortrade//oms/instruments/trading_pair.h"
#include "tensortrade/core/dexceptions.h"
#include "sstream"

namespace ttc {

    TradingPair::TradingPair(const Instrument& _base, const Instrument& _quote) : m_base(_base),
    m_quote(_quote) {
        if (m_base == m_quote) {
            throw InvalidTradingPair(m_base, m_quote);
        }
    }

    size_t TradingPair::TradingPair::hash() const {
        return std::hash<string>()(this->str()) ;
    }

    string TradingPair::str() const{

        std::stringstream ss;
        ss  << m_base.symbol() << "/" << m_quote.symbol();
        return ss.str();
    }

    bool TradingPair::operator==(TradingPair const &other) const {
        if (this->str() == other.str()) {
            return true;
        }
        return false;
    }

    bool TradingPair::operator!=(TradingPair const &other) const {
        return not operator==(other);
    }

}