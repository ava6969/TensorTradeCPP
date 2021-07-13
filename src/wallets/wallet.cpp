//
// Created by dewe on 7/1/21.
//

#include "tensortrade/oms/wallets/wallet.h"
#include "tensortrade/oms/orders/order.h"
#include "tensortrade/oms/exchanges/exchange.h"
#include "tensortrade/core/dexceptions.h"
#include "tensortrade/core/context.h"

namespace ttc
{

    Quantity Wallet::lock(Quantity quantity, const Order &order, const std::string &reason)
    {
        if(quantity.is_locked())
            throw DoubleLockedQuantity(quantity);

        if(quantity > m_balance)
        {
            if (quantity - m_balance > std::pow(10.f, -m_instrument.precision() + 2))
            {
                throw InsufficientFunds(m_balance, quantity);
            }else
            {
                quantity = m_balance;
            }
        }

        m_balance -= quantity;

        quantity = quantity.lock_for(order.pathID());

        if(not m_locked.contains(quantity.path_id.value()))
        {
            m_locked[quantity.path_id.value()] = quantity;
        }else
            m_locked[quantity.path_id.value()] += quantity;

        m_locked[quantity.path_id.value()] = m_locked[quantity.path_id.value()].quantize();
        m_balance = m_balance.quantize();

        std::stringstream src_stream, tgt_stream, memo;
        src_stream << m_exchange_id<< ":" << m_instrument.str() << "/free";
        tgt_stream << m_exchange_id << ":" << m_instrument.str() << "/locked";
        memo << "LOCK (" << reason << ")";
        Ctx::ctx()->ledger()->commit(*this, quantity, src_stream.str(), tgt_stream.str(), memo.str());

        return quantity;

    }

    Quantity Wallet::unlock(Quantity quantity, const string &reason) {
        if(not quantity.is_locked())
            throw DoubleUnlockedQuantity(quantity);

        if(not m_locked.contains(quantity.path_id.value()))
        {
            throw QuantityNotLocked(quantity);
        }

        if(quantity > m_locked[quantity.path_id.value()] )
        {
            throw InsufficientFunds(m_balance, quantity);
        }

        m_locked[quantity.path_id.value()] -= quantity;
        m_balance += quantity.free();

        m_locked[quantity.path_id.value()] = m_locked[quantity.path_id.value()].quantize();
        m_balance = m_balance.quantize();

        std::stringstream src_stream, tgt_stream, memo;
        src_stream << m_exchange_id << ":" << m_instrument.str() << "/locked";
        tgt_stream << m_exchange_id << ":" << m_instrument.str() << "/free";
        memo << "UNLOCK " << m_instrument << "(" << reason << ")";
        Ctx::ctx()->ledger()->commit(*this, quantity, src_stream.str(), tgt_stream.str(), memo.str());

        return quantity;
    }

    Quantity Wallet::deposit(Quantity quantity, const string &reason) {
        if(quantity.is_locked())
        {
            if(not m_locked.contains(quantity.path_id.value()))
            {
                m_locked[quantity.path_id.value()] = quantity;
            }else
            {
                m_locked[quantity.path_id.value()] += quantity;
            }
        }else
        {
            m_balance += quantity;
        }

        m_balance = m_balance.quantize();

        std::stringstream tgt_stream, memo;
        tgt_stream << m_exchange_id << ":" << m_instrument.str() << "/locked";
        memo << "DEPOSIT " << "(" << reason << ")";
        Ctx::ctx()->ledger()->commit(*this, quantity, m_exchange_id, tgt_stream.str(), memo.str());

        return quantity;
    }

    Quantity Wallet::withdraw(Quantity quantity, const string &reason) {

        if( quantity.is_locked())
        {
            auto ptr = m_locked.find(quantity.path_id.value());
            if (ptr != m_locked.end())
            {
                auto locked_quantity = ptr->second;
                if(quantity > locked_quantity)
                {
                    if(quantity - locked_quantity > std::pow(10.f, -m_instrument.precision() +2))
                    {
                        throw InsufficientFunds(locked_quantity, quantity);
                    }else
                        quantity = locked_quantity;
                }
            }
            m_locked[quantity.path_id.value()] -= quantity;
        }
        else if(not quantity.is_locked())
        {
            if(quantity > m_balance)
            {
                if(quantity - m_balance > std::pow(10.f, - m_instrument.precision() + 2))
                {
                    throw InsufficientFunds(m_balance, quantity);
                }else
                    quantity = m_balance;
            }
            m_balance -= quantity;
        }

        m_balance = m_balance.quantize();

        std::stringstream src_stream, memo;
        src_stream << m_exchange_id << ":" << m_instrument.str() << "/locked";
        memo << "WITHDRAWAL (" << reason << ")";

        Ctx::ctx()->ledger()->commit(*this,
                                     quantity,
                                     src_stream.str(),
                                     m_exchange_id,
                                     memo.str());

        return quantity;

    }

     Transfer Wallet::transfer(Wallet& source,
                               Wallet& target,
                               Quantity quantity,
                               Quantity commission,
                               ExchangePair const& exchange_pair,
                               const string &reason) {

         quantity = quantity.quantize();
         commission = commission.quantize();

         auto pair = source.m_instrument / target.m_instrument;
         auto poid = quantity.path_id;

         auto lsb1 = source.m_locked.at(poid.value()).size;
         auto ltb1 = target.m_locked.contains(poid.value()) ? target.m_locked.at(poid.value()).size :
                     (pair.quote() * 0.f).size;

         commission = source.withdraw(commission, "COMMISSION");
         quantity = source.withdraw(quantity, "FILL ORDER");

         Instrument instrument;
         float converted_size;
         if(quantity.instrument == exchange_pair.pair().base())
         {
             instrument = exchange_pair.pair().quote();
             converted_size = quantity.size / exchange_pair.price();
         }else
         {
             instrument = exchange_pair.pair().base();
             converted_size = quantity.size * exchange_pair.price();
         }

         auto converted = Quantity{instrument, converted_size, quantity.path_id}.quantize();

         std::stringstream ss;
         ss << "TRADED " << quantity.str() << " " << exchange_pair.str() << " @ "<< exchange_pair.price();
         converted = target.deposit(converted, ss.str());

         auto lsb2 = source.m_locked.at(poid.value()).size;
         auto ltb2 = target.m_locked.contains(poid.value())? target.m_locked.at(poid.value()).size :
                 (pair.quote()*0).size;

         auto q = quantity.size;
         auto c = commission.size;
         auto cv = converted.size;
         auto p = pair == exchange_pair.pair() ? exchange_pair.inverse_price() : exchange_pair.price();

         return {quantity, commission, exchange_pair.price()};

    }

    Wallet &Wallet::wallet(const string &wallet_id)
    {
        return Ctx::ctx()->wallet(wallet_id);
    }

    Wallet::Wallet(std::string  exchange_id, const Quantity &balance) :
            m_exchange_id(std::move(exchange_id)),
            initial_size(balance.size),
            m_instrument(balance.instrument),
            m_balance(balance.quantize()),
            m_locked({})
            {}


    Exchange Wallet::exchange() const {
        return Ctx::ctx()->exchanges(exchange_id());
    }

    std::ostream& operator<<(std::ostream& os, Wallet const& w)
    {
        os << w.str();
        return os;
    }


}
