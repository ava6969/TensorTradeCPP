//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_QUANTITY_H
#define TENSORTRADECPP_QUANTITY_H

#include <variant>
#include <sstream>
#include "string"
#include "optional"
#include "../../core/types.h"
#include "type_traits"
#include "tuple"
#include "instrument.h"
#include <cmath>

#define BOOL_OP(sign)  bool operator sign (T other) { \
    auto[left, right] = Quantity<T>::validate(this, other); \
    return left.size sign right.size; } \
    \
    bool operator sign (Quantity<T> other) { \
    auto[left, right] = Quantity<T>::validate(this, &other); \
    return left.size sign right.size; }

#define MATH_OP(sign)     Quantity<T> operator sign (Quantity<T>  other) { \
auto[left, right] = validate(this, &other); \
return Quantity<T>{left.instrument, left.size sign right.size, left.path_id }; } \
\
Quantity<T> operator sign (T  other) { \
auto[left, right] = validate(this, other); \
return Quantity<T>{left.instrument, left.size sign right.size }; }


#define IMATH_OP(sign)     Quantity<T>& operator sign (Quantity<T>  other) { \
auto[left, right] = validate(this, &other); \
*this = Quantity<T>{left.instrument, left.size sign right.size, left.path_id };  \
return *this; }                                                                            \
\
Quantity<T>& operator sign (T  other) { \
auto[left, right] = validate(this, other); \
*this = Quantity<T>{left.instrument, left.size sign right.size }; \
return *this; }

#define MATH_OP_DECLARE(sign)      friend Quantity<float> operator sign (float decimal, Quantity<float> self); \
friend Quantity<int> operator sign (int decimal, Quantity<int> self);


#define MATH_OP_DEFINE(sign)     Quantity<float> operator sign (float decimal, Quantity<float> self) { \
auto[left, right] = Quantity<float>::validate(decimal, &self); \
return {left.instrument, left.size - right.size }; }

namespace ttc {

    using std::string;

    template<typename T>
    struct Quantity {

        std::optional<string> path_id;
        Instrument instrument;
        T size;
        bool isLocked{false};

        Quantity(Instrument const& _instrument,
                 T const& _size,
                 std::optional<string>  _path_id = std::nullopt);

        Quantity()=default;
        Quantity(Quantity const &)=default;
        explicit Quantity(Quantity* _clone){path_id = _clone->path_id; instrument = _clone->instrument; size = _clone->size; };
        Quantity& operator=(Quantity const&)=default;

        [[nodiscard]] bool is_locked() { isLocked = path_id.has_value();  return isLocked;}

        inline Quantity<T> lock_for(string const& path_id)
        {
            return Quantity<T>(instrument, size, path_id);
        }

        [[nodiscard]] Quantity<float> convert(struct ExchangePair const& ) const;

        inline Quantity<T> free()
        {
            return Quantity<T>(instrument, size);
        }

        [[nodiscard]] inline Quantity<float> quantize() const
        {
            return {instrument, scalbln(size, powl(10, -instrument.precision)), path_id};
        }

        inline float asFloat() const { return float(size); }

        template<class K>
        inline Quantity<K> as() { return {instrument, K(size), path_id};}

        [[nodiscard]] Quantity<T> contain(ExchangePair* exchangePair) const;

        static std::pair<Quantity<T>, Quantity<T>> validate(Quantity<T>* left, Quantity<T>* right);

        static auto validate(T left, Quantity<T>* right) -> std::pair<Quantity<T>, Quantity<T>>
        {
            auto _left = Quantity<T>{right->instrument, left, right->path_id};
            Quantity<T> R = *right;
            return {_left, R};
        }
        static auto validate(Quantity<T>* left, T right)  -> std::pair<Quantity<T>, Quantity<T>>
        {
            auto _right = Quantity<T>{left->instrument, right, left->path_id};
            Quantity<T> L = *left;
            return {L, _right};
        }


        MATH_OP_DECLARE(-);
        MATH_OP_DECLARE(*);

        MATH_OP(+);
        MATH_OP(-);

     Quantity<T> operator*(Quantity<T>  other)
     {
        auto[left, right] = validate(this, &other);
        return Quantity<T>{left.instrument, left.size * right.size, left.path_id };
     }

     Quantity<T> operator* (T  other) {
        auto[left, right] = validate(this, other);
        return Quantity<T>{left.instrument, left.size * right.size, left.path_id };
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
            return instrument.symbol;
        }

    };
}
#endif //TENSORTRADECPP_QUANTITY_H
