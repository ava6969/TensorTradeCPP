//
// Created by dewe on 6/30/21.
//

#include "instrument.h"
#include "quantity.h"
#include "trading_pair.h"

namespace ttc
{

    Instrument::Instrument(string _symbol, int _precision, string _name):
            symbol(move(_symbol)),
            precision(_precision),
            name(move(_name))
    {}

    bool Instrument::operator==(Instrument const& other) const
    {
        return symbol == other.symbol and precision == other.precision and name == other.name;
    }

    bool Instrument::operator!=(Instrument const& other) const
    {
        return not operator==(other);
    }

    Quantity<float> Instrument::operator*(float other) const
    {
        return Quantity<float>{*this, other};
    }

    Quantity<int> Instrument::operator*(int other) const {
        return Quantity<int>(*this, other);
    }

    TradingPair Instrument::operator/(Instrument const& other) const
    {
        return TradingPair{*this, other};
    }

    size_t Instrument::hash() const {
        return std::hash<string>()(symbol);
    }

    string Instrument::str() const {
        return symbol;
    }

    std::ostream& operator<<(std::ostream& os, const Instrument &r) {
        os << r.str();
        return os;
    }

    Quantity<int> operator*(int other, Instrument const& self)  {
        return {self, other};
    }

    Quantity<float> operator*(float other, const Instrument &self) {
        return {self, other};
    }

}