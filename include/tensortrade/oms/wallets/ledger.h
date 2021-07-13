//
// Created by dewe on 7/1/21.
//

#ifndef TENSORTRADECPP_LEDGER_H
#define TENSORTRADECPP_LEDGER_H

#include "tensortrade/oms/instruments/quantity.h"
#include "wallet.h"
#include "tensortrade/oms/exchanges/exchange.h"
#include "tensortrade/core/context.h"

namespace ttc
{

    struct Transaction
    {
        string poid;
        int step;
        string source, target, memo;
        Quantity amount;
        Quantity free;
        Quantity locked;
        std::optional<Quantity> locked_poid;

    };
    using Transactions = std::vector<Transaction>;

    class Ledger
    {
        Transactions transactions;


    public:

        Ledger()=default;

        inline void commit(Wallet const& wallet, Quantity const& quantity,
                           string const& source, string const& target, string const& memo)
        {
            auto&& poid = quantity.path_id.value_or("");

            auto&& is_qty_locked = wallet.Locked().find(poid) == wallet.Locked().end() || poid.empty();

            auto locked_balance =  is_qty_locked ? std::nullopt : std::optional<Quantity>(wallet.Locked().at(poid));

            auto const& exchange = Ctx::ctx()->exchanges(wallet.exchange_id());

            transactions.emplace_back(Transaction{poid, exchange._clock->Step(),
                                      source,
                                      target,
                                      memo,
                                      quantity,
                                      wallet.Balance(),
                                      wallet.lockedBalance(),
                                      locked_balance});
        }

        void reset()
        {
            transactions.clear();
        }

    };


}


#endif //TENSORTRADECPP_LEDGER_H
