//
// Created by dewe on 6/30/21.
//

#include "dexceptions.h"
#include "instruments/instrument.h"
#include "instruments/quantity.h"

#define S(msg) std::string(msg)
namespace ttc
{

    InvalidTradingPair::InvalidTradingPair(Instrument const& base, Instrument const& quote):
    base(base),quote(quote){}

    const char *InvalidTradingPair::what() const noexcept {
        return string("Invalid Instrument pair ").append(base.str()).append("/").append(quote.str()).c_str();
    }

    const char *InvalidNegativeQuantity::what() const noexcept {
            return string( "Invalid Quantity: ").append(std::to_string(_size)).append(". Amounts cannot be negative.").c_str();
    }

    template<typename T>
    const char* IncompatibleInstrumentOperation<T>::what() const noexcept {
            return string( "Instruments are not of the same type (").
            append(left.str()).append(" and ").append(right.str()).append(").").c_str();
    }

    const char* QuantityOpPathMismatch::what() const noexcept {
        return string("Invalid operation between quantities with unequal path id: ").
                        append(left_id).append(" ").append(right_id).c_str();
    }

    const char* InvalidNumericQuantity::what() const noexcept {

        return string("Invalid Quantity: ").append(std::to_string(size)).
        append(". Amounts cannot be non numeric").c_str();
    }

    const char* DoubleLockedQuantity::what() const noexcept {
        return string("Cannot lock quantity that has previously been locked: ").append(q1.str()).c_str();
    }

    DoubleLockedQuantity::DoubleLockedQuantity(const Quantity<float>& q1):q1(q1) {

    }

    const char* DoubleUnlockedQuantity::what() const noexcept {
        return string("Cannot unlock quantity that has previously been unlocked: ").append(q1.str()).c_str();
    }

    DoubleUnlockedQuantity::DoubleUnlockedQuantity(Quantity<float> q1) :q1(q1){

    }

    const char *InvalidOrderQuantity::what() const noexcept {
        return string("Invalid Quantity: ").append(quantity.str()).append(" Order sizes must be positive.").c_str();
    }

    const char *InsufficientFunds::what() const noexcept
    {
        return S("Insufficient funds for allocating size").append(sz.str()).append(" with balance ").append(q1.str()).c_str();
    }

    InsufficientFunds::InsufficientFunds(const Quantity<float> &q1, const Quantity<float> &sz):q1(q1), sz(sz) {}

    template<typename T>
    const char *QuantityNotLocked<T>::what() const noexcept {
        return S("Cannot unlock quantity that has not been locked in this wallet: ").append(q1.str()).c_str();
    }

    template<typename T>
    QuantityNotLocked<T>::QuantityNotLocked(Quantity<T> q1):q1(q1) {

    }

    template class IncompatibleInstrumentOperation<int>;
    template class IncompatibleInstrumentOperation<float>;
    template class QuantityNotLocked<int>;
    template class QuantityNotLocked<float>;
};