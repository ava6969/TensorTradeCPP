//
// Created by dewe on 7/1/21.
//

#ifndef TENSORTRADECPP_WALLET_H
#define TENSORTRADECPP_WALLET_H

#include "instruments/quantity.h"
#include "../../core/base.h"

namespace ttc
{
    class Exchange;
    using WalletTuple = std::tuple<Exchange*, Instrument, int>;
    struct Transfer
    {
        Quantity<float> quantity;
        Quantity<float> commission;
        float price;
    };

    class Wallet : public Identifiable {

        static class Ledger* m_ledger;
        Instrument m_instrument;
        Quantity<float> m_balance;
        std::unordered_map<string, Quantity<float>> m_locked;
        int initial_size;
        Exchange* m_exchange;

    public:

        explicit Wallet(Exchange* _exchange, Quantity<int> const& balance);
        explicit Wallet(std::tuple<Exchange*, Instrument, int> const& wallet_tuple):
        Wallet(std::get<0>(wallet_tuple), Quantity<int>{std::get<1>(wallet_tuple), std::get<2>(wallet_tuple)}){}

        static Ledger& ledger();
        static void releaseLedger();

        auto instrument() const { return m_instrument;}

        inline Quantity<int> lockedBalance() const{
            auto locked_balance = Quantity<int>{m_instrument, 0};
            for(auto const&[key, quantity] : m_locked)
            {
                locked_balance += quantity.size;
            }
            return locked_balance;
        }

        Exchange* exchange() const;

        auto totalBalance() const{

            auto total_balance = m_balance;
            for(auto const&[key, quantity] : m_locked)
            {
                total_balance += quantity.size;
            }
            return total_balance;
        }

        /// The current quantities that are locked for orders.
        [[nodiscard]] inline auto const& Locked() const { return m_locked; }

        inline void removeLocked(std::string const& id)
        {
            m_locked.erase(id);
        }

        [[nodiscard]] inline auto  Balance() const { return m_balance; }

        Quantity<float> lock(Quantity<float> quantity, class Order const& order, string const& reason);
        Quantity<float> unlock(Quantity<float> quantity, string const& reason);

        Quantity<float> deposit(Quantity<float> quantity, string const& reason);
        Quantity<float> withdraw(Quantity<float> quantity, string const& reason);

        static Transfer transfer(Wallet* source,
                                  Wallet* target,
                                  Quantity<float> quantity,
                                  Quantity<float> commission,
                                  ExchangePair* exchange_pair, const string &reason);
        void reset()
        {
            m_balance = Quantity<float>{m_instrument, initial_size}.quantize();
            m_locked.clear();
        }

        [[nodiscard]] string str() const{
            return string("<Wallet: balance=").append(m_balance.str()).append(", locked=").append(lockedBalance().str());
        }


    };
}



#endif //TENSORTRADECPP_WALLET_H
