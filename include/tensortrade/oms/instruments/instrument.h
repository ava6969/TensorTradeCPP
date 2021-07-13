//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_INSTRUMENT_H
#define TENSORTRADECPP_INSTRUMENT_H

#include <optional>
#include "string"
#include "tensortrade/core/types.h"

namespace ttc
{

    using std::string;

    struct Quantity;

    class Instrument
    {
        string m_symbol{};
        int m_precision{};
        string m_name{};

    public:

        Instrument()=default;
        Instrument(Instrument const&)=default;
        Instrument& operator=(Instrument const&)=default;
        Instrument(string _symbol, int _precision, string _name="");

        bool operator==(Instrument const& other) const;
        bool operator!=(Instrument const& other) const;
        auto operator+(Instrument const& other) const
        {
            return Instrument(m_symbol + other.m_symbol, m_precision + other.m_precision,
                              m_name + other.m_name);
        }

        Quantity operator*(double other) const;
        friend Quantity operator*(double other, Instrument const& self);

        struct TradingPair operator/(Instrument const& other) const;
        friend std::ostream& operator<<(std::ostream& os, Instrument const& r);

        [[nodiscard]] size_t hash() const;
        [[nodiscard]] string str() const;

        [[nodiscard]] string symbol() const { return m_symbol; }
        [[nodiscard]] int precision() const { return m_precision; }
        [[nodiscard]] string name() const { return m_name; }

    };

    auto static const BTC = Instrument{"BTC", 8, "Bitcoin"};
    auto static const ETH = Instrument{"ETH", 8, "Ethereum"};
    auto static const XRP = Instrument{"XRP", 8, "XRP"};
    auto static const BCH = Instrument("BCH", 8, "Bitcoin Cash");
    auto static const LTC = Instrument("LTC", 8, "Litecoin");

    auto static const USD = Instrument{"USD", 2, "U.S. Dollar"};
    auto static const EUR = Instrument{"EUR", 2, "Euro"};

    // Stocks
    auto static const AAPL = Instrument("AAPL", 2, "Apple stock");
    auto static const MSFT = Instrument("MSFT", 2, "Microsoft stock");
    auto static const TSLA = Instrument("TSLA", 2, "Tesla stock");
    auto static const AMZN = Instrument("AMZN", 2, "Amazon stock");

}
#endif //TENSORTRADECPP_INSTRUMENT_H
