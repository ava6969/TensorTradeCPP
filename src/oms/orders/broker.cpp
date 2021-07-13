//
// Created by dewe on 6/30/21.
//

#include "tensortrade/oms/orders/broker.h"
#include "tensortrade/oms/orders/order.h"
#include "tensortrade/oms/orders/trade.h"
#include "tensortrade/oms/exchanges/exchange.h"
#include "tensortrade/oms/wallets/wallet.h"

namespace ttc
{
    void Broker::cancel(Order& order) {

        if (order.Status() == OrderStatus::CANCELLED) {
            std::cerr << "Order " << order.ID() << "has already been cancelled.";
        }

        std::erase_if(unexecuted, [&order](auto const &x) { return x == order; });

        order.cancel();
    }

    void Broker::submit(Order const& order)
    {
        unexecuted.push_back(order);
    }

    void Broker::update()
    {
         vector<string> executed_ids;
        for(auto& order : unexecuted)
        {
            if(order.isExecutable())
            {
                auto  key = order.ID();
                executed_ids.push_back(key);
                executed.insert_or_assign(key, order);
                executed.at(key).attach(this);
                executed.at(key).execute();
            }
        }

        for(auto const& order_id : executed_ids)
        {
            erase(unexecuted, executed.at(order_id));
        }

        for(auto& order : unexecuted)
        {
            if(order.isActive() and order.isExpired())
            {
                cancel(order);
            }
        }

        for(auto &[k, values] : executed)
        {
            if(values.isActive() and values.isExpired())
            {
                cancel(values);
            }
        }

    }

    void Broker::onFill(Order* order, Trade* trade)         {

        for(auto& t: trades)
        {
            if(find(t.second.begin(), t.second.end(), trade) != t.second.end())
            {
                return;
            }
        }

        if(executed.contains(trade->orderID()))
        {
            trades[trade->orderID()] = trades.contains(trade->orderID()) ? trades[trade->orderID()] : vector<Trade*>{};
            trades[trade->orderID()].push_back(trade);

            if(order->isComplete())
            {
                std::optional<Order> next_order = order->complete();

                if(next_order)
                {
                    if(next_order->isExecutable())
                    {
                        auto const& key = next_order->ID();
                        executed.insert_or_assign(key, next_order.value());
                        executed.at(key).attach(this);
                        executed.at(key).execute();
                    }
                    else
                    {
                        submit(next_order.value());
                    }
                }
            }
        }
    }
}

