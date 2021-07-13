//
// Created by dewe on 6/30/21.
//


#include "tensortrade/oms/orders/order.h"
#include "tensortrade/core/clock.h"
#include "tensortrade/oms/exchanges/exchange.h"
#include "tensortrade/oms/wallets/wallet.h"
#include "tensortrade/oms/instruments/quantity.h"
namespace ttc
{

    std::optional<Order>  createOrder(Order& order, OrderSpec const& spec)
    {
        auto wallet_instrument = instrument(spec.side, spec.exchange_pair.pair());

        Exchange const& exchange = Ctx::ctx()->exchanges(order.exchangePair().exchange_name());
        auto& wallet = order.portfolio().getWallet(exchange.ID(), wallet_instrument);
        auto quantity = wallet.Locked().contains(order.pathID()) ?
        std::optional<Quantity>(wallet.Locked().at(order.pathID())) : std::nullopt;

        if( not quantity or quantity->size == 0)
            return std::nullopt;

        return Order{exchange._clock->Step(),
                     spec.side, spec.type, spec.exchange_pair,
                     quantity, order.portfolio(),
                     spec.exchange_pair.price(), order.pathID(),
                     std::nullopt, order.End(), std::move(spec._criteria)};
    }

}