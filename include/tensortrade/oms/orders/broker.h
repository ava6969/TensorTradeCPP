//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_BROKER_H
#define TENSORTRADECPP_BROKER_H

#include "tensortrade/core/base.h"
#include "order_listener.h"
#include "vector"
#include "string"
#include "unordered_map"
#include "map"
#include "vector"
#include "order.h"

using std::vector;
using std::string;
class Trade;

namespace ttc {

    class Broker : public OrderListener, public TimeIndexed {

    private:
        std::vector<Order> unexecuted{};
        std::unordered_map<string, Order> executed{};
        std::map<string, vector<Trade*>> trades{};

    public:

        Broker() = default;

        void submit(Order const&order);
        void cancel(Order& order);
        void update();
        void onFill(Order* order, Trade* trade) override;

        void reset()
        {
            unexecuted.clear();
            executed.clear();
            trades.clear();
        }

    };

}

#endif //TENSORTRADECPP_BROKER_H
