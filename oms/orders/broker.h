//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_BROKER_H
#define TENSORTRADECPP_BROKER_H

#include "../../core/base.h"
#include "order_listener.h"
#include "vector"
#include "string"
#include "unordered_map"
#include "map"
#include "vector"

using std::vector;
using std::string;
class Order;
class Trade;

namespace ttc {

    class Broker : public OrderListener, public TimeIndexed {

    private:
        std::vector<std::shared_ptr<Order>> unexecuted{};
        std::unordered_map<string, std::shared_ptr<Order>> executed{};
        std::map<string, vector<Trade*>> trades{};

    public:

        Broker() = default;

        void submit(std::shared_ptr<Order> const& order);
        void cancel(Order* order);
        void update();
        void onFill(Order* order, Trade* trade);

        void reset()
        {
            unexecuted.clear();
            executed.clear();
            trades.clear();
        }

    };

}

#endif //TENSORTRADECPP_BROKER_H
