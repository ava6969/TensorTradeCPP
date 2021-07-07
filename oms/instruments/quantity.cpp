//
// Created by dewe on 6/30/21.
//

#include <cmath>
#include "quantity.h"
#include "exchange_pair.h"
#include "trading_pair.h"
#include "exchanges/exchange.h"
#include "../../core/dexceptions.h"


namespace ttc {

    template<typename T>
    Quantity<T>::Quantity(Instrument const &_instrument, const T &_size,std::optional<string> _path_id):
    instrument(_instrument), size(_size),
    path_id(std::move(_path_id))
    {

        if (size < 0) {
            if (std::abs(size) > std::pow(10, -instrument.precision)) {
                throw InvalidNegativeQuantity(size);
            } else {
                size = 0;
            }
        }
    }
    template<> Quantity<float> Quantity<float>::convert(ExchangePair const &exchange_pair) const {

        float converted_size;
        Instrument _instrument;
        if (instrument == exchange_pair.pair().base) {
            _instrument = exchange_pair.pair().quote;
            converted_size = asFloat() / exchange_pair.price();
        } else {
            _instrument = exchange_pair.pair().base;
            converted_size = asFloat() * exchange_pair.price();
        }

        return Quantity<float>{_instrument, converted_size, path_id};
    }

    template<typename T>
    Quantity<T> Quantity<T>::contain(ExchangePair *exchangePair) const {
        auto options = exchangePair->exchange()->Options();
        auto price = exchangePair->price();

        if (exchangePair->pair().base == instrument) {
            return Quantity<T>{instrument, std::min<float>(size, options.max_trade_size), path_id};
        }

        auto _size = size * price;
        if (_size < options.max_trade_size) {
            return Quantity<T>{instrument, size, path_id};
        }

        auto max_trade_size = options.max_trade_size;
        auto contained_size = max_trade_size / price;
        contained_size = std::round(std::scalbln(contained_size, std::pow(10, -instrument.precision)));
        return Quantity<T>{instrument, contained_size, path_id};

    }

    template<typename T>
    std::pair<Quantity<T>, Quantity<T>> Quantity<T>::validate(Quantity<T> *left, Quantity<T> *right) {
        if (left->instrument != right->instrument) {
            throw IncompatibleInstrumentOperation{*left, *right};
        }

        if ((left->path_id && right->path_id) && (left->path_id != right->path_id)) {
            throw QuantityOpPathMismatch(left->path_id.value_or("null"),
                                         right->path_id.value_or("null"));
        } else if (left->path_id && not right->path_id) {
            right->path_id = left->path_id;
        } else if (not left->path_id && right->path_id) {
            left->path_id = right->path_id;
        }

        return std::pair<Quantity<T>, Quantity<T>>{*left, *right};
    }

    MATH_OP_DEFINE(-);

    MATH_OP_DEFINE(*);

    template class Quantity<int>;
    template class Quantity<float>;
}