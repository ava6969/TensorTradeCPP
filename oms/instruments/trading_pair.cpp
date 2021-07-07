//
// Created by dewe on 6/30/21.
//

#include "trading_pair.h"
#include "../../core/dexceptions.h"
#include "sstream"

namespace ttc {

    TradingPair::TradingPair(const Instrument& _base, const Instrument& _quote) : base(_base),
    quote(_quote) {
        if (base == quote) {
            throw InvalidTradingPair(base, quote);
        }
    }

    size_t TradingPair::TradingPair::hash() const {
        return std::hash<string>()(this->str()) ;
    }

    string TradingPair::str() const{

        std::stringstream ss;
        ss  << base.symbol << "/" << quote.symbol;
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