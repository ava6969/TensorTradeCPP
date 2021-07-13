//
// Created by dewe on 6/30/21.
//

#include <cmath>
#include "tensortrade/oms/instruments/quantity.h"
#include "tensortrade/oms/instruments/exchange_pair.h"
#include "tensortrade/oms/instruments/trading_pair.h"
#include "tensortrade/oms/exchanges/exchange.h"
#include "tensortrade/core/dexceptions.h"


namespace ttc {


    Quantity::Quantity(Instrument const &_instrument, const double &_size,std::optional<string> _path_id):
    instrument(_instrument), size(_size),
    path_id(std::move(_path_id)),
    TEN_PRECISION(std::pow(10, instrument.precision()))
    {

        if (size < 0) {
            if (std::abs(size) > std::pow(10, -instrument.precision())) {
                throw InvalidNegativeQuantity(size);
            } else {
                size = 0;
            }
        }
    }

    Quantity Quantity::convert(ExchangePair const &exchange_pair) const {

        double converted_size;
        Instrument _instrument;
        if (instrument == exchange_pair.pair().base()) {
            _instrument = exchange_pair.pair().quote();
            converted_size = this->asDouble() / exchange_pair.price();
        } else {
            _instrument = exchange_pair.pair().base();
            converted_size = this->asDouble() * exchange_pair.price();
        }

        return Quantity{_instrument, converted_size, path_id};
    }


    Quantity Quantity::contain(const ExchangePair &exchangePair) const {

        auto options = Exchange::exchange(exchangePair).Options();
        auto price = exchangePair.price();

        if (exchangePair.pair().base() == instrument) {
            return Quantity{instrument, std::min<double>(size, options.max_trade_size), path_id};
        }

        auto _size = size * price;
        if (_size < options.max_trade_size) {
            return Quantity{instrument, size, path_id};
        }

        auto max_trade_size = options.max_trade_size;
        auto contained_size = max_trade_size / price;

        contained_size = floor(contained_size * TEN_PRECISION) / TEN_PRECISION;
        return Quantity{instrument, contained_size, path_id};

    }


    std::pair<Quantity, Quantity> Quantity::validate(Quantity left, Quantity right){

        if (left.instrument != right.instrument) {
            throw IncompatibleInstrumentOperation{left, right};
        }
        if ((left.path_id && right.path_id) && (left.path_id != right.path_id)) {
            throw QuantityOpPathMismatch(left.path_id.value_or("null"), right.path_id.value_or("null"));
        }
        else if (left.path_id && not right.path_id) {
            right.path_id = left.path_id;
        }
        else if (not left.path_id && right.path_id) {
            left.path_id = right.path_id;
        }

        return {std::move(left), move(right)};
    }

    MATH_OP_DEFINE(-);
    MATH_OP_DEFINE(*)

    std::ostream& operator<<(std::ostream& os, Quantity q) {
        os << q.str();
        return os;
    }

    double Quantity::asDouble() const {
        return size;
    };

}