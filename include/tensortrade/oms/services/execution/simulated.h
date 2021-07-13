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

        [[nodiscard]]  std::optional<class Trade> executeBuyOrder(class Order& order, class Wallet& baseWallet,
                Wallet & quoteWallet, float currentPrice,
                struct ExchangeOptions const& options, const class Clock* clock) const;

        [[nodiscard]]  std::optional<Trade> executeSellOrder(Order& order, Wallet& baseWallet,
                                                              Wallet & quoteWallet,
                                                    float currentPrice, ExchangeOptions const& options,
                                                    const Clock* clock) const;


        std::optional<Trade>  operator()(Order& order, Wallet& baseWallet, Wallet& quoteWallet,
                float currentPrice, ExchangeOptions const& options, const Clock* clock) const;

        std::optional<Trade>  operator()(Order& order, Wallet&& baseWallet, Wallet&& quoteWallet,
                                         float currentPrice, ExchangeOptions const& options, const Clock* clock) const;

    };


}
