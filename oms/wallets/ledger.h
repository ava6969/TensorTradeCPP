//
// Created by dewe on 7/1/21.
//

#ifndef TENSORTRADECPP_LEDGER_H
#define TENSORTRADECPP_LEDGER_H

#include "instruments/quantity.h"
#include "xtensor/xrandom.hpp"
#include "xframe/xio.hpp"
#include "xframe/xvariable.hpp"
#include "wallet.h"
#include "instruments/quantity.h"

namespace ttc
{

    using coordinate_type = xf::xcoordinate<xf::fstring>;

    struct Transaction
    {
        string poid;
        int step;
        string source, target, memo;
        Quantity<float> amount;
        Quantity<float> free;
        Quantity<int> locked;
        std::optional<Quantity<float>> locked_poid;

    };

    using Transactions = std::vector<Transaction>;

    class Ledger
    {
        Transactions transactions;

    public:

        Ledger()=default;

        inline void commit(Wallet const& wallet, Quantity<float> const& quantity,
                           string const& source, string const& target, string const& memo)
        {
            auto poid = quantity.path_id.value_or("");
            auto locked_poid_balance = wallet.Locked().find(poid) == wallet.Locked().end()
                    || poid.empty() ? std::nullopt : std::optional<Quantity<float>>(wallet.Locked().at(poid));

            transactions.emplace_back(Transaction{poid,
                                      wallet.exchange()->_clock.Step(),
                                      source,
                                      target,
                                      memo,
                                      quantity,
                                      wallet.Balance(),
                                      wallet.lockedBalance(),
                                      locked_poid_balance});

        }

        coordinate_type asFrame(bool sort_by_order_seq= false)
        {

        }

        void reset()
        {
            transactions = {};
        }

    };


}


#endif //TENSORTRADECPP_LEDGER_H
