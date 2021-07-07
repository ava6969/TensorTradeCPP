//
// Created by dewe on 6/30/21.
//

#include "criteria.h"
#include "order.h"
#include "exchanges/exchange.h"

namespace ttc
{
    CriteriaBinOp Criteria::operator& (Criteria * other) {
        return CriteriaBinOp(this, other, [](bool x, bool y) { return x & y; }, "&");
    }

    CriteriaBinOp Criteria::operator| (Criteria* other) {
        return CriteriaBinOp(this, other, [](bool x, bool y) { return x | y; }, "|");
    }

    CriteriaBinOp Criteria::operator^ (Criteria* other) {
        return CriteriaBinOp(this, other, [](bool x, bool y) { return x ^ y; }, "^");
    }

    NotCriteria Criteria::operator!()  {
        return NotCriteria(*this);
    }

    bool Criteria::operator()(const Order *order, const Exchange * exchange) const
    {
        if(not exchange->isPairTradable(order->pair()))
        {
            return false;
        }
        return check(order, exchange);
    }

    bool CriteriaBinOp::check(const Order * order, const Exchange * exchange) const
    {
        bool l = left->operator()(order, exchange);
        bool r = right->operator()(order, exchange);
        return this->op(l, r);
    }

    bool NotCriteria::check(const Order *order, const Exchange *exchange) const         {
        return not criteria(order, exchange);
    }

    bool Limit::check(const Order * order, const Exchange * exchange) const
    {
        auto price = exchange->quote_price(order->pair());

        auto buy_satisfied = order->Side() == TradeSide::BUY and price <= limit_price;
        auto sell_satisfied = order->Side() == TradeSide::SELL and price >= limit_price;

        return buy_satisfied or sell_satisfied;
    }

    bool Stop::check(const Order *order, const Exchange * exchange) const         {
        auto price = exchange->quote_price(order->pair());
        auto _percent = abs(price - order->Price()) / order->Price();

        auto isTakeProfit = dir == up and price >= order->Price();
        auto isStopLoss = dir == down and price <= order->Price();

        return (isTakeProfit or isStopLoss) and _percent >= this->percent;
    }

    bool Timed::check(const Order * order, const Exchange * exchange) const
    {
        return (order->_clock.Step() - order->createdAt() ) <= m_duration;
    }

}

