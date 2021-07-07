//
// Created by dewe on 6/30/21.
//

#include "broker.h"
#include "orders/order.h"
#include "trade.h"
#include "exchanges/exchange.h"
#include "wallets/wallet.h"

namespace ttc
{
    void Broker::cancel(Order *order) {

        if (order->Status() == OrderStatus::CANCELLED) {
            std::cerr << "Order " << order->ID() << "has already been cancelled.";
        }

        std::erase_if(unexecuted, [&order](auto const &x) { return x.get() == order; });

        order->cancel();
    }

    void Broker::submit(std::shared_ptr<Order> const& order)  { unexecuted.emplace_back(order); }

    void Broker::update()
    {
        vector<string> executed_ids;
        for(auto const& order : unexecuted)
        {
            if(order->isExecutable())
            {
                auto const& key = order->ID();
                executed_ids.push_back(key);
                executed[key] = order;
                executed[key]->attach(this);
                executed[key]->execute();
            }
        }

        for(auto const& order_id : executed_ids)
        {
            erase(unexecuted, executed[order_id]);
        }

        for(auto const &order : unexecuted)
        {
            if(order->isActive() and order->isExpired())
            {
                cancel(order.get());
            }
        }
        for(auto const &[k, values] : executed)
        {
            if(values->isActive() and values->isExpired())
            {
                cancel(values.get());
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
                std::shared_ptr<Order> next_order = order->complete();

                if(next_order)
                {
                    if(next_order->isExecutable())
                    {
                        auto const& key = next_order->ID();
                        executed[key] = next_order;
                        executed[key]->attach(this);
                        executed[key]->execute();
                    }
                    else
                    {
                        submit(next_order);
                    }
                }
            }
        }
    }
}

