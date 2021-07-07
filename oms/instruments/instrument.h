//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_INSTRUMENT_H
#define TENSORTRADECPP_INSTRUMENT_H

#include <optional>
#include "string"
#include "../../core/types.h"

namespace ttc
{

    using std::string;

    template<typename T>
    struct Quantity;

    struct Instrument
    {

        string symbol{};
        int precision{};
        string name{};

        Instrument()=default;
        Instrument(Instrument const&)=default;
        Instrument& operator=(Instrument const&)=default;
        Instrument(string _symbol, int _precision, string _name="");

        bool operator==(Instrument const& other) const;
        bool operator!=(Instrument const& other) const;
        auto operator+(Instrument const& other) const
        {
            return Instrument(symbol + other.symbol, precision + other.precision,
                              name + other.name);
        }

        struct Quantity<float> operator*(float other) const;
        Quantity<int> operator*(int other) const;
        friend Quantity<int> operator*(int other, Instrument const& self);
        friend Quantity<float> operator*(float other, Instrument const& self);

        struct TradingPair operator/(Instrument const& other) const;
        friend std::ostream& operator<<(std::ostream& os, Instrument const& r);

        [[nodiscard]] size_t hash() const;
        [[nodiscard]] string str() const;

    };

    auto static const BTC = Instrument{"BTC", 8, "Bitcoin"};
    auto static const ETH = Instrument{"ETH", 8, "Bitcoin"};

    auto static const USD = Instrument{"USD", 2, "U.S. Dollar"};
    auto static const EUR = Instrument{"EUR", 2, "Euro"};

    // Stocks
    auto static const AAPL = Instrument("AAPL", 2, "Apple stock");
    auto static const MSFT = Instrument("MSFT", 2, "Microsoft stock");
    auto static const TSLA = Instrument("TSLA", 2, "Tesla stock");
    auto static const AMZN = Instrument("AMZN", 2, "Amazon stock");

}
#endif //TENSORTRADECPP_INSTRUMENT_H
