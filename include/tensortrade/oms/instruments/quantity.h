//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_QUANTITY_H
#define TENSORTRADECPP_QUANTITY_H

#include <variant>
#include <sstream>
#include "string"
#include "optional"
#include "tensortrade/core/types.h"
#include "type_traits"
#include "tuple"
#include "instrument.h"
#include <cmath>
#include<bits/stdc++.h>

#define BOOL_OP(sign)  bool operator sign (double other) { \
    auto[left, right] = Quantity::validate(*this, other); \
    return left.size sign right.size; } \
    \
    bool operator sign (Quantity other) { \
    auto[left, right] = Quantity::validate(*this, other); \
    return left.size sign right.size; }

#define MATH_OP(sign)     Quantity operator sign (Quantity  other) { \
auto[left, right] = validate(*this, other); \
return Quantity{left.instrument, left.size sign right.size, left.path_id }; } \
\
Quantity operator sign (double  other) { \
auto[left, right] = validate(*this, other); \
return Quantity{left.instrument, left.size sign right.size }; }


#define IMATH_OP(sign)     Quantity& operator sign (Quantity  other) { \
auto[left, right] = validate(*this, other); \
*this = Quantity{left.instrument, left.size sign right.size, left.path_id };  \
return *this; }                                                                            \
\
Quantity& operator sign (double  other) { \
auto[left, right] = validate(*this, other); \
*this = Quantity{left.instrument, left.size sign right.size }; \
return *this; }

#define MATH_OP_DECLARE(sign)      \
    friend Quantity operator sign (double decimal, Quantity self);

#define MATH_OP_DEFINE(sign)     \
    Quantity operator sign (double decimal, Quantity self) { \
            auto[left, right] = Quantity::validate(decimal, self); \
            return {left.instrument, left.size - right.size };                \
            }

namespace ttc {

    using std::string;

    struct Quantity {

        std::optional<string> path_id;
        Instrument instrument;
        double size;
        bool isLocked{false};
        int TEN_PRECISION = std::pow(10, instrument.precision());

        Quantity(Instrument const& _instrument,
                 double const& _size,
                 std::optional<string>  _path_id = std::nullopt);

        Quantity()=default;
        Quantity(Quantity const &)=default;

        Quantity& operator=(Quantity const&)=default;

        [[nodiscard]] inline bool is_locked() { isLocked = path_id.has_value();  return isLocked;}

        inline auto lock_for(string const& _path_id)
        {
            return Quantity(instrument, size, _path_id);
        }

        [[nodiscard]] Quantity convert(struct ExchangePair const& ) const;

        inline auto free()
        {
            return Quantity(instrument, size);
        }

        [[nodiscard]] inline Quantity quantize() const
        {
            return {instrument,  size, path_id};
        }

        [[nodiscard]] Quantity contain(const ExchangePair& exchangePair) const;

        static std::pair<Quantity, Quantity> validate(Quantity  left, Quantity right);

        static auto validate(double left, Quantity right) -> std::pair<Quantity, Quantity>
        {
            auto _left = Quantity{right.instrument, left, right.path_id};
            return {_left, std::move(right)};
        }

        static auto validate(Quantity left, double right)  -> std::pair<Quantity, Quantity>
        {
            auto _right = Quantity{left.instrument, right, left.path_id};
            return {std::move(left), _right};
        }

        MATH_OP_DECLARE(-);
        MATH_OP_DECLARE(*);
        MATH_OP(+);
        MATH_OP(-);

     Quantity operator*(Quantity  other)
     {
        auto[left, right] = validate(*this, other);
        return Quantity{left.instrument, left.size * right.size, left.path_id };
     }

     Quantity operator* (double  other) {
        auto[left, right] = validate(*this, other);
        return Quantity{left.instrument, left.size * right.size, left.path_id };
     }

        IMATH_OP(+=);
        IMATH_OP(-=);

        BOOL_OP(==);
        BOOL_OP(<);
        BOOL_OP(>);
        BOOL_OP(>=);
        BOOL_OP(<=);

        [[nodiscard]] string str()const
        {
            return instrument.symbol();
        }

        friend std::ostream& operator<<(std::ostream& os, Quantity q);


        double asDouble() const;
    };
}

#endif //TENSORTRADECPP_QUANTITY_H
