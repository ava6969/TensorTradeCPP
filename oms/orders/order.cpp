//
// Created by dewe on 6/30/21.
//

#include "order.h"

#include <utility>
#include "order_spec.h"
#include "instruments/exchange_pair.h"
#include "wallets/portfolio.h"
#include "exchanges/exchange.h"
#include "wallets/wallet.h"
#include "../../core/dexceptions.h"
namespace ttc
{

    void Order::execute() {
        status = OrderStatus::OPEN;

        if(m_portfolio->orderListener())
        {
            this->attach(m_portfolio->orderListener());
        }

        for (auto listener : listeners)
            listener->onExecute(this);

        m_exchange_pair.exchange()->executeOrder(this, m_portfolio.get());
    }

    void Order::fill(std::unique_ptr<Trade> trade) {

        status = OrderStatus::PARTIALLY_FILLED;

        auto filled = trade->quantity() + trade->commission();
        m_remaining.value() -= filled;
        trades.push_back(std::move(trade));

        for(OrderListener* listener : listeners)
        {
            listener->onFill(this, trades.back().get());
        }

    }

    bool Order::isComplete() const {

        if(status == OrderStatus::CANCELLED)
            return true;

        auto wallet = m_portfolio->getWallet(m_exchange_pair.exchange()->ID(),
                                             instrument(side, m_exchange_pair.pair()));
        auto quantity = path_id.empty() ? std::nullopt : std::optional<Quantity<float>>(wallet->Locked().at(path_id));

        return (quantity.has_value() && quantity->size == 0 ) or m_remaining->size <= 0;

    }

    void Order::release(const string &reason){

        for(auto& wallet : m_portfolio->wallets())
        {
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

    std::unique_ptr<Order> Order::complete() {
        status = OrderStatus::FILLED;
        std::unique_ptr<Order> order;

        if(not specs.empty())
        {
            auto order_spec = std::move(specs.back());
            specs.pop_back();
            order = createOrder(this, &order_spec);
        }

        for(OrderListener* listener : listeners)
        {
            listener->onComplete(this);
        }

        if(not order)
            release("COMPLETE");
        return order;
    }

    Portfolio* Order::portfolio() { return m_portfolio.get(); }
    ExchangePair* Order::exchangePair()  { return &(m_exchange_pair); }
    const Quantity<float>* Order::remaining() const { return &m_remaining.value(); }
    TradingPair const& Order::pair() const { return m_exchange_pair.pair();}

    float Order::size()
    {
        if( not m_quantity.has_value())
        {
            return -1;
        }
        return m_quantity->size;
    }

    Instrument const& Order::baseInstrument() const { return m_exchange_pair.pair().base;}
    Instrument const& Order::quoteInstrument() const { return m_exchange_pair.pair().quote;}

    bool Order::isExecutable() const{
        auto is_satisfied = not criteria or criteria->operator()(this, m_exchange_pair.exchange());
        auto clock = m_exchange_pair.exchange()->_clock;
        return is_satisfied and clock.Step() >= start;
    }

    bool Order::isExpired() const {
        return end ? m_exchange_pair.exchange()->_clock.Step() >= end : false;
    }

    Order *Order::addOrderSpec(OrderSpec order_spec)
    {
        specs.emplace_back(std::move(order_spec));
        return this;
    }

    Order::Order(int step, const TradeSide &side, const TradeType &trade_type, ExchangePair  _exchange_pair,
                 optional <Quantity<float>> _quantity, Portfolio *_portfolio, float price, const string &path_id,
                 std::optional<int> start, std::optional<int> end,
                 std::shared_ptr<Criteria> _criteria) :side(side),
                 step(step), trade_type(trade_type), m_exchange_pair(std::move(_exchange_pair)),
                 m_quantity(_quantity), m_portfolio(_portfolio), price(price), path_id(path_id), start(start),
                 end(end), criteria(move(_criteria))
                 {

        _quantity = _quantity->contain(&m_exchange_pair);
        if(_quantity->size == 0)
            throw InvalidOrderQuantity{_quantity.value()};

        boost::mt19937 ran;
        boost::uuids::basic_random_generator<boost::mt19937> gen(&ran);
        auto data = gen();
        auto uuid = std::string(data.begin(), data.end());

        this->path_id = path_id.empty() ? uuid : path_id;

        auto wallet = _portfolio->getWallet(m_exchange_pair.exchange()->ID(), instrument(side, m_exchange_pair.pair()));

        if(not wallet->Locked().contains(path_id))
        {
            this->m_quantity = wallet->lock(*_quantity, *this, "lock for order");
        }

        m_remaining = this->m_quantity;
    }


}