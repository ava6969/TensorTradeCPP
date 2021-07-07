//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_ORDER_SPEC_H
#define TENSORTRADECPP_ORDER_SPEC_H

#include "core/base.h"
#include "instruments/exchange_pair.h"
#include "instruments/instrument.h"
#include "trade.h"
#include "criteria.h"

namespace ttc {

    struct OrderSpec : public Identifiable {
        TradeSide side;
        TradeType type;
        ExchangePair exchange_pair;
        std::shared_ptr<Criteria> _criteria;

        OrderSpec(TradeSide side, TradeType type, ExchangePair const& exchangePair,
                  std::shared_ptr<Criteria> _criteria):side(side), type(type),
                  exchange_pair(exchangePair), _criteria(_criteria) {}
    };

    std::unique_ptr<Order> createOrder(class Order* order, OrderSpec* spec);

    static inline std::map<string, std::any> to_dict( OrderSpec const& spec)
    {
        return {{"id", spec.ID()},
                {"type", spec.type},
                {"exchange_pair", spec.exchange_pair.str()},
//                {"criteria", spec._criteria}
        };
    }

    static inline string str( OrderSpec const& spec)
    {
        std::stringstream ss;
        ss << "Trade: ";
        for(auto const&[k, v] : to_dict(spec))
        {
            ss << k << "=" << any_cast<string>(v) << ", ";
        }
        return ss.str();
    }

}
#endif //TENSORTRADECPP_ORDER_SPEC_H
