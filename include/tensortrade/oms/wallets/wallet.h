//
// Created by dewe on 7/1/21.
//

#ifndef TENSORTRADECPP_WALLET_H
#define TENSORTRADECPP_WALLET_H

#include "tensortrade/oms/instruments/quantity.h"
#include "tensortrade/core/base.h"

namespace ttc
{
    class Exchange;
    using WalletTuple = std::tuple<Exchange*, Instrument, int>;
    struct Transfer
    {
        Quantity quantity;
        Quantity commission;
        double price;
    };

    class Wallet : public Identifiable {

        Instrument m_instrument;
        Quantity m_balance;
        std::unordered_map<string, Quantity> m_locked;
        double initial_size;
        std::string m_exchange_id;

    public:

        explicit Wallet( std::string  m_exchange_id, Quantity const& balance);
        Wallet(Wallet const&)=default;
        Wallet& operator=(Wallet const&)=default;
        Wallet(std::tuple< std::string , Instrument, double> const& wallet_tuple):
        Wallet(std::get<0>(wallet_tuple),
                Quantity{std::get<1>(wallet_tuple),
                        std::get<2>(wallet_tuple)}){}

        auto instrument() const { return m_instrument;}
        inline std::string const& exchange_id() const { return m_exchange_id; }

        inline Quantity lockedBalance() const{
            auto locked_balance = Quantity{m_instrument, 0};
            for(auto const&[key, quantity] : m_locked)
            {
                locked_balance += quantity.size;
            }
            return locked_balance;
        }

        [[nodiscard]] Exchange exchange() const;

        static Wallet& wallet(string const& wallet_id);

        auto totalBalance() const{

            auto total_balance = m_balance;
            for(auto const&[key, quantity] : m_locked)
            {
                total_balance += (quantity.size);
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

        [[nodiscard]] Quantity lock(Quantity quantity, class Order const& order, string const& reason);
        [[nodiscard]]  Quantity unlock(Quantity quantity, string const& reason);

        [[nodiscard]] Quantity deposit(Quantity quantity, string const& reason);
        [[nodiscard]] Quantity withdraw(Quantity quantity, string const& reason);

        [[nodiscard]]  static Transfer transfer(Wallet& source, Wallet& target,
                                  Quantity quantity,
                                  Quantity commission,
                                  ExchangePair const& exchange_pair,
                                  const string &reason);
        void reset()
        {
            m_balance = Quantity{m_instrument, initial_size}.quantize();
            m_locked.clear();
        }

        [[nodiscard]] string str() const{
            return string("<Wallet: balance=").append(m_balance.str()).append(", locked=").append(lockedBalance().str());
        }

        friend std::ostream& operator<<(std::ostream& os, Wallet const& w);



    };
}



#endif //TENSORTRADECPP_WALLET_H
