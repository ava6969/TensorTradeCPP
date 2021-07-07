//
// Created by dewe on 6/30/21.
//


#include "order.h"
#include "../../core/clock.h"
#include "exchanges/exchange.h"
#include "wallets/wallet.h"
#include "instruments/quantity.h"
namespace ttc
{

    std::unique_ptr<Order> createOrder(Order* order, OrderSpec * spec)
    {
        auto wallet_instrument = instrument(spec->side, spec->exchange_pair.pair());

        Exchange* exchange = order->exchangePair()->exchange();
        auto wallet = order->portfolio()->getWallet(exchange->ID(), wallet_instrument);
        auto quantity = wallet->Locked().contains(order->pathID()) ?
        std::optional<Quantity<float>>(wallet->Locked().at(order->pathID())) : std::nullopt;

        if( not quantity or quantity->size == 0)
            return nullptr;

        return std::make_unique<Order>(exchange->_clock.Step(),
                     spec->side, spec->type, spec->exchange_pair,
                     quantity, order->portfolio(),
                     spec->exchange_pair.price(), order->pathID(),
                     std::nullopt, order->End(), std::move(spec->_criteria));
    }

}