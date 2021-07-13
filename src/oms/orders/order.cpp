//
// Created by dewe on 6/30/21.
//

#include "tensortrade/oms/orders/order.h"

#include <utility>
#include "tensortrade/oms/orders/order_spec.h"
#include "tensortrade/oms/instruments/exchange_pair.h"
#include "tensortrade/oms/wallets/portfolio.h"
#include "tensortrade/oms/exchanges/exchange.h"
#include "tensortrade/oms/wallets/wallet.h"
#include "tensortrade/core/dexceptions.h"
namespace ttc
{

    void Order::execute() {
        status = OrderStatus::OPEN;

        if(m_portfolio.orderListener())
        {
            this->attach(m_portfolio.orderListener());
        }

        for (auto listener : listeners)
            listener->onExecute(this);

        auto const& exchange = Exchange::exchange(m_exchange_pair);
        exchange.executeOrder(*this, m_portfolio);
    }

    void Order::fill(Trade const& trade) {

        status = OrderStatus::PARTIALLY_FILLED;

        auto filled = trade.quantity() + trade.commission();
        m_remaining.value() -= filled;
        trades.emplace_back(trade);

        for(OrderListener* listener : listeners)
        {
            listener->onFill(this, std::addressof(trades.back()));
        }

    }

    bool Order::isComplete() {

        if(status == OrderStatus::CANCELLED)
            return true;

        auto& wallet = m_portfolio.getWallet(Ctx::ctx()->exchanges(m_exchange_pair.exchange_id()).ID(),
                                             instrument(side, m_exchange_pair.pair()));
        auto quantity = path_id.empty() ? std::nullopt : std::optional<Quantity>(wallet.Locked().at(path_id));

        return (quantity.has_value() && quantity->size == 0 ) or m_remaining->size <= 0;

    }

    void Order::release(const string &reason){

        for(auto& wallet_id : m_portfolio.wallets())
        {
            auto& wallet = Wallet::wallet(wallet_id);
            if(wallet.Locked().contains(path_id))
            {
                auto _quantity = wallet.Locked().at(path_id);

                wallet.unlock(_quantity, reason);

                wallet.removeLocked(path_id);
            }
        }
    }

    void Order::cancel(const string &reason) {
        this->status = OrderStatus::CANCELLED;

        for(OrderListener* listener : listeners)
        {
            listener->onCancel(this);
        }

        listeners = {};
        release(reason);
    }

    std::optional<Order>  Order::complete() {
        status = OrderStatus::FILLED;
        std::optional<Order>  order;

        if(not specs.empty())
        {
            auto order_spec = std::move(specs.back());
            specs.pop_back();
            order = createOrder(*this, order_spec);
        }

        for(OrderListener* listener : listeners)
        {
            listener->onComplete(this);
        }

        if(not order)
            release("COMPLETE");
        return order;
    }

    Portfolio& Order::portfolio()  { return m_portfolio; }
    ExchangePair const& Order::exchangePair()  const { return m_exchange_pair; }
    Quantity Order::remaining() const { return m_remaining.value(); }
    TradingPair Order::pair() const { return m_exchange_pair.pair();}

    double Order::size()
    {
        if( not m_quantity.has_value())
        {
            return -1;
        }
        return m_quantity->size;
    }

    Instrument Order::baseInstrument() const { return m_exchange_pair.pair().base();}
    Instrument Order::quoteInstrument() const { return m_exchange_pair.pair().quote();}

    bool Order::isExecutable() const{
        auto const& exchange = Exchange::exchange(m_exchange_pair);
        auto const& order = *this;
        auto is_satisfied = not criteria or criteria.value()(order, exchange);
        return is_satisfied and exchange._clock->Step() >= start;
    }

    bool Order::isExpired() const {
        return end ?    Exchange::exchange(m_exchange_pair)._clock->Step() >= end : false;
    }

    void Order::addOrderSpec(OrderSpec order_spec)
    {
        specs.emplace_back(std::move(order_spec));
    }

    Order::Order(int step,
                 TradeSide const& side,
                 TradeType const& trade_type,
                 ExchangePair const& _exchange_pair,
                 optional <Quantity> _quantity,
                 Portfolio  _portfolio,
                 double price, const string &path_id,
                 std::optional<int> start,
                 std::optional<int> end,
                 const std::optional<std::function<bool(Order const& order, Exchange const&)>>& _criteria):
                 side(side),
                 step(step),
                 trade_type(trade_type),
                 m_exchange_pair(_exchange_pair),
                 m_quantity(_quantity),
                 m_portfolio(std::move(_portfolio)),
                 price(price),
                 path_id(path_id),
                 start(start),
                 end(end),
                 criteria(_criteria)
                 {

        _quantity = _quantity->contain(m_exchange_pair);
        if(_quantity->size == 0)
            throw InvalidOrderQuantity{_quantity.value()};

         boost::uuids::random_generator generator;
         boost::uuids::uuid uuid1 = generator();
         std::stringstream ss;
         ss << uuid1;
         auto uuid = ss.str();

        this->path_id = path_id.empty() ? uuid : path_id;

        auto& wallet = m_portfolio.getWallet(m_exchange_pair.exchange_id(), instrument(side, m_exchange_pair.pair()));

        if(not wallet.Locked().contains(path_id))
        {
            this->m_quantity = wallet.lock(*_quantity, *this, "lock for order");
        }

        m_remaining = this->m_quantity;
    }


}