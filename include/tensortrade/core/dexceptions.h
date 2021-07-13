//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_DEXCEPTIONS_H
#define TENSORTRADECPP_DEXCEPTIONS_H

#include "dexceptions.h"
#include "exception"
#include "string"
#include "tensortrade/oms/instruments/quantity.h"

namespace ttc
{
    using std::exception;
    using std::string;
    struct Instrument;


    struct InvalidTradingPair : public exception
    {
        Instrument const& base;
        Instrument const& quote;

        InvalidTradingPair(Instrument const& base, Instrument const& quote);

        [[nodiscard]] string msg() const noexcept;

        ~InvalidTradingPair() noexcept override = default;

    };

    struct InvalidNegativeQuantity : public exception
    {
        double _size;
        explicit InvalidNegativeQuantity(double size):_size(size){}

        [[nodiscard]] string msg() const noexcept;

        ~InvalidNegativeQuantity() noexcept override = default;
    };


    struct IncompatibleInstrumentOperation : public exception
    {
        Quantity const& left;
        Quantity const& right;
        IncompatibleInstrumentOperation(Quantity const& left, Quantity const& right):left(left), right(right) {}

        [[nodiscard]] string msg() const noexcept;

        ~IncompatibleInstrumentOperation() noexcept override = default;
    };

    struct QuantityOpPathMismatch : public exception
    {
        string const& left_id;
        string const& right_id;
        QuantityOpPathMismatch(string const& left_id, string const& right_id):left_id(left_id), right_id(right_id){}

        [[nodiscard]] string msg() const noexcept;

        ~QuantityOpPathMismatch() noexcept override = default;
    };

    struct InvalidNumericQuantity : public exception
    {
        double size;
        explicit InvalidNumericQuantity(double const& size):size(size){}

        [[nodiscard]] string msg() const noexcept;

        ~InvalidNumericQuantity() noexcept override = default;
    };

    struct DoubleLockedQuantity : public exception
    {
        Quantity q1;
        explicit DoubleLockedQuantity(const Quantity& q1);

        [[nodiscard]] string msg() const noexcept;

        ~DoubleLockedQuantity() noexcept override = default;
    };

    struct InsufficientFunds : public exception
    {
        Quantity q1;
        Quantity sz;

        explicit InsufficientFunds(Quantity const& q1, Quantity const& sz);

        [[nodiscard]] string msg() const noexcept;

        ~InsufficientFunds() noexcept override = default;
    };

    struct DoubleUnlockedQuantity : public exception
    {
        Quantity q1;
        explicit DoubleUnlockedQuantity(Quantity q1);

        [[nodiscard]] string msg() const noexcept;

        ~DoubleUnlockedQuantity() noexcept override = default;
    };


    struct QuantityNotLocked : public exception
    {
        Quantity q1;
        explicit QuantityNotLocked(Quantity q1);

        [[nodiscard]] string msg() const noexcept;

        ~QuantityNotLocked() noexcept override = default;
    };

    struct InvalidOrderQuantity : public exception
    {
        Quantity const& quantity;
        explicit InvalidOrderQuantity(Quantity const& quantity):quantity(quantity){}

        [[nodiscard]] string msg() const noexcept;

        ~InvalidOrderQuantity() noexcept override = default;
    };

    struct InCompatibleExchangePair : public exception
    {
        const class Exchange& _exchange;
        TradingPair const& _pair;
        explicit InCompatibleExchangePair(const Exchange& _exchange, class TradingPair const& _pair);

        [[nodiscard]] string msg() const noexcept;

        ~InCompatibleExchangePair() noexcept override = default;
    };

}


#endif //TENSORTRADECPP_DEXCEPTIONS_H
