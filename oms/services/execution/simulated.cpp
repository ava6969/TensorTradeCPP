//
// Created by dewe on 7/3/21.
//

//
// Created by dewe on 7/2/21.
//

#ifndef TENSORTRADECPP_SIMULATED_H
#define TENSORTRADECPP_SIMULATED_H

#include "core/base.h"
#include "wallets/wallet.h"
#include "orders/order.h"
#include "simulated.h"
#include "orders/trade.h"
#include "exchanges/exchange.h"

namespace ttc
{

        std::unique_ptr<Trade> Service::executeBuyOrder(Order* order, Wallet* baseWallet, Wallet* quoteWallet,
                                                      float currentPrice, ExchangeOptions const& options,
                                                      Clock const& clock) const
        {
            if(order->type() == TradeType::LIMIT && order->Price() < currentPrice)
                return nullptr;

            auto filled = order->remaining()->contain(order->exchangePair());

            if(order->type() == TradeType::MARKET)
            {
                auto scale = order->Price() / std::max<float>(currentPrice, order->Price());
                filled = filled * scale;
            }

            Quantity commission = filled * options.commission;
            auto quantity = filled - commission;

            if(commission.size < std::pow(10, -quantity.instrument.precision) )
            {
                std::cerr << "Commission is less than instrument precision. Canceling order. "
                             "Consider defining a custom instrument with a higher precision.";
                order->cancel("COMMISSION IS LESS THAN PRECISION.");
                return nullptr;
            }

            auto transfer = Wallet::transfer(baseWallet, quoteWallet, quantity, commission,
                                             order->exchangePair(), "BUY");

            return std::make_unique<Trade>(TradeOption{order->ID(), clock.Step(),
                                                            order->exchangePair(),
                                                            TradeSide::BUY,
                                                            order->type(),
                                                            transfer.quantity,
                                                            transfer.price,
                                                            transfer.commission});
        }

        std::unique_ptr<Trade> Service::executeSellOrder(Order* order,
                                                       Wallet* baseWallet,
                                                       Wallet* quoteWallet, float currentPrice,
                                                       ExchangeOptions const& options, Clock const& clock) const
        {
            if(order->type() == TradeType::LIMIT && order->Price() > currentPrice)
                return nullptr;

            auto filled = order->remaining()->contain(order->exchangePair());

            Quantity commission = filled * options.commission;
            auto quantity = filled - commission;

            if(commission.size < std::pow(10, -quantity.instrument.precision) )
            {
                std::cerr << "Commission is less than instrument precision. Canceling order. "
                             "Consider defining a custom instrument with a higher precision.";
                order->cancel("COMMISSION IS LESS THAN PRECISION.");
                return nullptr;
            }

            auto transfer = Wallet::transfer(quoteWallet, baseWallet, quantity, commission, order->exchangePair(), "SELL");

            return std::make_unique<Trade>(TradeOption{order->ID(), clock.Step(), order->exchangePair(), TradeSide::SELL,
                                                            order->type(),
                                                            transfer.quantity, transfer.price, transfer.commission});
        }

        std::unique_ptr<Trade> Service::operator()(Order* order, Wallet* baseWallet, Wallet* quoteWallet,
                        float currentPrice, ExchangeOptions const& options, Clock const& clock) const
        {
            if(order->isBuy())
            {
                return executeBuyOrder(order, baseWallet, quoteWallet, currentPrice, options, clock);
            }else if(order->isSell())
            {
                return executeSellOrder(order, baseWallet, quoteWallet, currentPrice, options, clock);
            }else
            {
                return nullptr;
            }
        }

}


#endif //TENSORTRADECPP_SIMULATED_H
