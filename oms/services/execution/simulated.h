//
// Created by dewe on 7/2/21.
//

#pragma once
#include <optional>
#include "memory"

namespace ttc
{

    struct Service
    {

        [[nodiscard]] std::unique_ptr<class Trade> executeBuyOrder(class Order* order, class Wallet* baseWallet,
                Wallet*quoteWallet, float currentPrice,
                struct ExchangeOptions const& options, class Clock const& clock) const;

        [[nodiscard]] std::unique_ptr<Trade> executeSellOrder(Order* order, Wallet* baseWallet, Wallet* quoteWallet,
                                                    float currentPrice, ExchangeOptions const& options,
                                                    Clock const& clock) const;


        std::unique_ptr<Trade>  operator()(Order* order, Wallet* baseWallet, Wallet* quoteWallet,
                                 float currentPrice, ExchangeOptions const& options, Clock const& clock) const;

    };


}
