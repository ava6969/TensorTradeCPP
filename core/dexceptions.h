//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_DEXCEPTIONS_H
#define TENSORTRADECPP_DEXCEPTIONS_H

#include "dexceptions.h"
#include "exception"
#include "string"
#include "instruments/quantity.h"

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

        [[nodiscard]] const char* what() const noexcept override;

        ~InvalidTradingPair() noexcept override = default;

    };

    struct InvalidNegativeQuantity : public exception
    {
        float _size;
        explicit InvalidNegativeQuantity(float size):_size(size){}

        [[nodiscard]] const char* what() const noexcept override;

        ~InvalidNegativeQuantity() noexcept override = default;
    };

    template<typename T>
    struct IncompatibleInstrumentOperation : public exception
    {
        Quantity<T> const& left;
        Quantity<T> const& right;
        IncompatibleInstrumentOperation(Quantity<T> const& left, Quantity<T> const& right):left(left), right(right) {}

        [[nodiscard]] const char* what() const noexcept override;

        ~IncompatibleInstrumentOperation() noexcept override = default;
    };

    struct QuantityOpPathMismatch : public exception
    {
        string const& left_id;
        string const& right_id;
        QuantityOpPathMismatch(string const& left_id, string const& right_id):left_id(left_id), right_id(right_id){}

        [[nodiscard]] const char* what() const noexcept override;

        ~QuantityOpPathMismatch() noexcept override = default;
    };

    struct InvalidNumericQuantity : public exception
    {
        float size;
        explicit InvalidNumericQuantity(float const& size):size(size){}

        [[nodiscard]] const char* what() const noexcept override;

        ~InvalidNumericQuantity() noexcept override = default;
    };

    struct DoubleLockedQuantity : public exception
    {
        Quantity<float> q1;
        explicit DoubleLockedQuantity(const Quantity<float>& q1);

        [[nodiscard]] const char* what() const noexcept override;

        ~DoubleLockedQuantity() noexcept override = default;
    };

    struct InsufficientFunds : public exception
    {
        Quantity<float> q1;
        Quantity<float> sz;

        explicit InsufficientFunds(Quantity<float> const& q1, Quantity<float> const& sz);

        [[nodiscard]] const char* what() const noexcept override;

        ~InsufficientFunds() noexcept override = default;
    };

    struct DoubleUnlockedQuantity : public exception
    {
        Quantity<float> q1;
        explicit DoubleUnlockedQuantity(Quantity<float> q1);

        [[nodiscard]] const char* what() const noexcept override;

        ~DoubleUnlockedQuantity() noexcept override = default;
    };

    template<typename T>
    struct QuantityNotLocked : public exception
    {
        Quantity<T> q1;
        explicit QuantityNotLocked(Quantity<T> q1);

        [[nodiscard]] const char* what() const noexcept override;

        ~QuantityNotLocked() noexcept override = default;
    };

    struct InvalidOrderQuantity : public exception
    {
        Quantity<float> const& quantity;
        explicit InvalidOrderQuantity(Quantity<float> const& quantity):quantity(quantity){}

        [[nodiscard]] const char* what() const noexcept override;

        ~InvalidOrderQuantity() noexcept override = default;
    };

}


#endif //TENSORTRADECPP_DEXCEPTIONS_H
