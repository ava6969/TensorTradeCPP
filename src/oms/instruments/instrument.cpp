//
// Created by dewe on 6/30/21.
//

#include "tensortrade/oms/instruments/instrument.h"
#include "tensortrade/oms/instruments/quantity.h"
#include "tensortrade/oms/instruments/trading_pair.h"

namespace ttc
{

    Instrument::Instrument(string _symbol, int _precision, string _name):
            m_symbol(move(_symbol)),
            m_precision(_precision),
            m_name(move(_name))
    {}

    bool Instrument::operator==(Instrument const& other) const
    {
        return m_symbol == other.m_symbol and m_precision == other.m_precision and m_name == other.m_name;
    }

    bool Instrument::operator!=(Instrument const& other) const
    {
        return not operator==(other);
    }

    TradingPair Instrument::operator/(Instrument const& other) const
    {
        return TradingPair{*this, other};
    }

    size_t Instrument::hash() const {
        return std::hash<string>()(m_symbol);
    }

    string Instrument::str() const {
        return m_symbol;
    }

    std::ostream& operator<<(std::ostream& os, const Instrument &r) {
        os << r.str();
        return os;
    }

    Quantity operator*(double other, const Instrument &self) {
        return Quantity{self, other};
    }

    Quantity Instrument::operator*(double other) const {
        return Quantity(*this, other);
    }

}