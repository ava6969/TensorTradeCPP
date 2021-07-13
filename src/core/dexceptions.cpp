//
// Created by dewe on 6/30/21.
//

#include "tensortrade/core/dexceptions.h"
#include "tensortrade/oms/instruments/instrument.h"
#include "tensortrade/oms/instruments/trading_pair.h"
#include "tensortrade/oms/exchanges/exchange.h"

#define S(msg) std::string(msg)
namespace ttc
{

    InvalidTradingPair::InvalidTradingPair(Instrument const& base, Instrument const& quote):
    base(base),quote(quote){}

    string InvalidTradingPair::msg() const noexcept {
        return string("Invalid Instrument pair ").append(base.str()).append("/").append(quote.str());
    }

    string InvalidNegativeQuantity::msg() const noexcept {
            return string( "Invalid Quantity: ").append(std::to_string(_size)).append(". Amounts cannot be negative.");
    }

    string IncompatibleInstrumentOperation::msg() const noexcept {
            return string( "Instruments are not of the same type (").
            append(left.str()).append(" and ").append(right.str()).append(").");
    }

    string QuantityOpPathMismatch::msg() const noexcept {
        return string("Invalid operation between quantities with unequal path id: ").
                        append(left_id).append(" ").append(right_id);
    }

    string InvalidNumericQuantity::msg() const noexcept {

        return string("Invalid Quantity: ").append(std::to_string(size)).
        append(". Amounts cannot be non numeric").c_str();
    }

    string DoubleLockedQuantity::msg() const noexcept {
        return string("Cannot lock quantity that has previously been locked: ").append(q1.str());
    }

    DoubleLockedQuantity::DoubleLockedQuantity(const Quantity& q1):q1(q1) {

    }

    string DoubleUnlockedQuantity::msg() const noexcept {
        return string("Cannot unlock quantity that has previously been unlocked: ").append(q1.str());
    }

    DoubleUnlockedQuantity::DoubleUnlockedQuantity(Quantity q1) :q1(q1){

    }

    string InvalidOrderQuantity::msg() const noexcept {
        return string("Invalid Quantity: ").append(quantity.str()).append(" Order sizes must be positive.");
    }

    string InsufficientFunds::msg() const noexcept
    {
        return S("Insufficient funds for allocating size").append(sz.str()).append(" with balance ").append(q1.str());
    }

    InsufficientFunds::InsufficientFunds(const Quantity &q1, const Quantity &sz):q1(q1), sz(sz) {}

    string QuantityNotLocked::msg() const noexcept {
        return S("Cannot unlock quantity that has not been locked in this wallet: ").append(q1.str()).c_str();
    }

    QuantityNotLocked::QuantityNotLocked(Quantity q1):q1(q1) {

    }

    InCompatibleExchangePair::InCompatibleExchangePair(const Exchange &_exchange, const TradingPair &_pair):
    _exchange(_exchange), _pair(_pair) {}

    string InCompatibleExchangePair::msg() const noexcept {
        return S("Exchnage").append(_exchange.Name()).
        append("is incompatible with trading pair").append(_pair.str()).c_str();
    }
};