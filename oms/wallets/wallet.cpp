//
// Created by dewe on 7/1/21.
//

#include "wallet.h"
#include "orders/order.h"
#include "exchanges/exchange.h"
#include "../../core/dexceptions.h"
#include "ledger.h"

namespace ttc
{

    Ledger*  Wallet::m_ledger = new Ledger();

    Quantity<float> Wallet::lock(Quantity<float> quantity, const Order &order, const std::string &reason)
    {
        if(quantity.is_locked())
            throw DoubleLockedQuantity(quantity);

        if(quantity > m_balance)
        {
            if (quantity - m_balance > std::pow(10, -m_instrument.precision + 2))
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
        src_stream << m_exchange->Name() << ":" << m_instrument.str() << "/free";
        tgt_stream << m_exchange->Name() << ":" << m_instrument.str() << "/locked";
        memo << "LOCK (" << reason << ")";
        m_ledger->commit(*this, quantity, src_stream.str(), tgt_stream.str(), memo.str());

        return quantity;

    }

    Quantity<float> Wallet::unlock(Quantity<float> quantity, const string &reason) {
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
        src_stream << m_exchange->Name() << ":" << m_instrument.str() << "/locked";
        tgt_stream << m_exchange->Name() << ":" << m_instrument.str() << "/free";
        memo << "UNLOCK " << m_instrument << "(" << reason << ")";
        m_ledger->commit(*this, quantity, src_stream.str(), tgt_stream.str(), memo.str());

        return quantity;
    }

    Quantity<float> Wallet::deposit(Quantity<float> quantity, const string &reason) {
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
        tgt_stream << m_exchange->Name() << ":" << m_instrument.str() << "/locked";
        memo << "DEPOSIT " << "(" << reason << ")";
        m_ledger->commit(*this, quantity, m_exchange->Name(), tgt_stream.str(), memo.str());

        return quantity;
    }

    Quantity<float> Wallet::withdraw(Quantity<float> quantity, const string &reason) {

        if( quantity.is_locked())
        {
            auto ptr = m_locked.find(quantity.path_id.value());
            if (ptr != m_locked.end())
            {
                auto locked_quantity = ptr->second;
                if(quantity > locked_quantity)
                {
                    if(quantity-locked_quantity > std::pow(10, -m_instrument.precision+2))
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
                if(quantity - m_balance > pow(10, - m_instrument.precision + 2))
                {
                    throw InsufficientFunds(m_balance, quantity);
                }else
                    quantity = m_balance;
            }
            m_balance -= quantity;
        }

        m_balance = m_balance.quantize();

        std::stringstream src_stream, memo;
        src_stream << m_exchange->Name() << ":" << m_instrument.str() << "/locked";
        memo << "WITHDRAWAL (" << reason << ")";

        m_ledger->commit(*this,
                      quantity,
                      src_stream.str(),
                      m_exchange->Name(),
                      memo.str());

        return quantity;

    }

     Transfer Wallet::transfer(Wallet* source,
                               Wallet* target,
                               Quantity<float> quantity,
                               Quantity<float> commission,
                               ExchangePair* exchange_pair, const string &reason) {

        quantity = quantity.quantize();
        commission = commission.quantize();

        auto pair = source->m_instrument / target->m_instrument;
        auto poid = quantity.path_id;

        auto lsb1 = source->m_locked[poid.value()].size;
        auto ltb1 = target->m_locked.contains(poid.value()) ? target->m_locked[poid.value()].size :
                (pair.quote * 0).size;

        commission = source->withdraw(commission, "COMMISSION");
        quantity = source->withdraw(quantity, "FILL ORDER");

        Instrument instrument;
        float converted_size;
        if(quantity.instrument == exchange_pair->pair().base)
        {
            instrument = exchange_pair->pair().quote;
            converted_size = quantity.size / exchange_pair->price();
        }else
        {
            instrument = exchange_pair->pair().base;
            converted_size = quantity.size * exchange_pair->price();
        }

        auto converted = Quantity{instrument, converted_size, quantity.path_id}.quantize();

        std::stringstream ss;
        ss << "TRADED " << quantity.str() << " " << exchange_pair->str() << " @ "<< exchange_pair->price();
        converted = target->deposit(converted, ss.str());

        auto lsb2 = source->m_locked[poid.value()].size;
        auto ltb2 = target->m_locked.contains(poid.value())? target->m_locked[poid.value()].size : (pair.quote*0).size;

        auto q = quantity.size;
        auto c = commission.size;
        auto cv = converted.size;
        auto p = pair == exchange_pair->pair() ? exchange_pair->inverse_price() : exchange_pair->price();

//        auto source_quantization = std::pow(10, -source->m_instrument.precision);
//        auto target_quantization = std::pow(10, -target->m_instrument.precision);
//
//        auto lhs = std::scalbln( (lsb1 - lsb2) - (q + c), source_quantization);
//        auto rhs = std::scalbln( (ltb2 - ltb1 - cv), target_quantization);
//
//        auto lhs_eq_zero = abs(lhs) <= source_quantization;
//        auto rhs_eq_zero = abs(rhs) <= source_quantization;
//
//        if( not lhs_eq_zero or not rhs_eq_zero)
//        {
//            // create equation
//            throw std::runtime_error("Invalid Transfer");
//        }

         return {quantity, commission, exchange_pair->price()};

    }

    Wallet::Wallet(class Exchange* _exchange, const Quantity<int> &balance) :
            m_exchange(_exchange),
            initial_size(balance.size),
            m_instrument(balance.instrument),
            m_balance(balance.quantize()),
            m_locked({})
            {}

    Exchange *Wallet::exchange() const { return m_exchange; }

    Ledger& Wallet::ledger() {
        return *m_ledger;
    }

    void Wallet::releaseLedger() { delete m_ledger; }

}
