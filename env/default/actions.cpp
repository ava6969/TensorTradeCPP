//
// Created by dewe on 7/5/21.
//

#include "actions.h"

#include <utility>
#include "wallets/portfolio.h"
#include "orders/order.h"
#include "orders/broker.h"
#include "wallets/wallet.h"
#include "orders/create.h"

namespace ttc
{
    TensorTradeActionScheme::TensorTradeActionScheme(Portfolio *portfolio, Broker *broker):
    m_portfolio(portfolio),
    m_broker(broker)
    {}

    void TensorTradeActionScheme::clock(const Clock &clock) {
        TimeIndexed::clock(clock);

        m_portfolio->_clock = clock;
        m_broker->_clock = clock;
        for(auto const& exchange: m_portfolio->exchanges())
            exchange->_clock = clock;
    }

    void TensorTradeActionScheme::perform(struct TradingEnv *env, const torch::Tensor &action) {
        auto orders = getOrders(action, m_portfolio);

        for(auto const& order: orders)
        {
            if(order)
            {
                //log
                m_broker->submit(order);
            }
        }

        m_broker->update();

    }

    Portfolio *TensorTradeActionScheme::portfolio() const {
        return m_portfolio;
    }

    std::vector<std::shared_ptr<class Order>> BSH::getOrders(torch::Tensor const& _action, Portfolio *portfolio)  {
        std::shared_ptr<Order> order;

        if(abs(_action.item<int>() - action) > 0)
        {
            const auto& src = action == 0 ? cash : asset;
            const auto& tgt = action == 0 ? asset : cash;

            if(src.Balance() == 0.f)
            {
                return {};
            }

            order = proportionOrder(portfolio, src, tgt, 1.0);
            this->action = _action.item<int>();
        }

        for(auto const& listener: listeners)
            listener->onAction({action});

        return {order};
    }


    BSH::BSH(Portfolio* portfolio, Broker* broker, const Wallet &cash, const Wallet &asset):
    TensorTradeActionScheme(portfolio, broker),cash(cash), asset(asset), action(0) {}

    SimpleOrders::SimpleOrders(Portfolio* portfolio, Broker* broker,
                               std::vector<std::shared_ptr< Criteria > > criteria,
                               std::vector<float> trade_sizes,
                               std::vector<int> durations, TradeType tradeType,
                               std::vector<OrderListener *> listeners,
                               float min_order_pct, float min_order_abs):
                               TensorTradeActionScheme(portfolio, broker),
                               m_criteria(std::move(criteria)),
                               type(tradeType),
                               m_listeners(std::move(listeners)),
                               min_order_abs(min_order_abs),
                               min_order_pct(min_order_pct),
                               m_duration(std::move(durations)),
                               m_trade_sizes(std::move(trade_sizes))
                               {}

    SimpleOrders::SimpleOrders(Portfolio* portfolio, Broker* broker,
                               std::vector<std::shared_ptr<Criteria > > criteria, int trade_size, std::vector<int> durations,
                               TradeType tradeType, std::vector<OrderListener *>  listeners, float min_order_pct,
                               float min_order_abs):
                                TensorTradeActionScheme(portfolio, broker),
                                m_criteria(std::move(criteria)),
                                type(tradeType),
                                m_listeners(std::move(listeners)),
                                min_order_abs(min_order_abs),
                                min_order_pct(min_order_pct),
                                m_duration(std::move(durations)){

        for (int i = 0; i < trade_size; ++i) {
            m_trade_sizes.push_back(float(i + 1) / trade_size );
        }

   }

    std::vector<std::shared_ptr<class Order>> SimpleOrders::getOrders(torch::Tensor const& _action,
            Portfolio *portfolio)
    {
        if(_action[1].item<int>() == 0)
            return {};

        vector<int> padded_action(5);
        padded_action[0] = _action[0].item<int>();
        padded_action[1] = _action[1].item<int>();
        padded_action[2] = _action[2].item<int>();
        padded_action[3] = action_types[3] == 0 ? -1 : action_types[3] == 1 ? 1 : _action[3].item<int>();
        padded_action[4] = action_types[4] == 0 ? -1 : action_types[4] == 1 ? 1 : _action[4].item<int>();

        auto ep = portfolio->exchangePairs()[padded_action[0]];
        auto side = padded_action[1] == 1 ? TradeSide::BUY : TradeSide::SELL;
        auto proportion = m_trade_sizes[padded_action[2]];

        auto instrument_ = instrument( side, ep.pair());

        auto&& wallet = portfolio->getWallet(ep.exchange()->ID(), instrument_);
        auto&& balance = wallet->Balance().asFloat();
        auto size = (balance * proportion);
        size = std::min<float>(balance, size);
//
//        std::unique_ptr<Criteria> _criteria {}; TODO: FIX LATER
//        if(padded_action[3] == -1 )
//            _criteria =  *(m_criteria[padded_action[3]]);

        auto duration = padded_action[4] == -1 ? std::nullopt :
                std::optional<int>(m_duration[padded_action[4]] + _clock.Step());

        auto quantity = (size * instrument_).quantize();

        if(size < std::pow(10, -instrument_.precision) or
        size < min_order_pct * portfolio->netWorth() or
        size < min_order_abs)
        {
            return {};
        }

//        auto order = std::make_shared<Order>(_clock.Step(),
//                                             side, type,
//                                             ep,
//                                             quantity, portfolio, ep.price(), "",
//                                             std::nullopt, duration, {});

        for(auto const& listener: listeners)
            listener->onAction(padded_action);

        return {};


    }

    std::vector<int> SimpleOrders::actionShape() {
        action_types = {this->m_portfolio->exchangePairs().size() + 1, 2, m_trade_sizes.size(), m_criteria.size(),
                        m_duration.size()};

        for(auto a : action_types)
        {
            if(a < 2)
                d_action_shape.push_back(int(a));
        }

        return d_action_shape;
    }

    ManagedRiskOrders::ManagedRiskOrders(Portfolio *portfolio, Broker *broker,
                                         std::vector<float> stop,
                                         std::vector<float> take,
                                         std::vector<float> trade_size, std::vector<int> durations, TradeType tradeType,
                                         std::vector<OrderListener *> listeners, float min_order_pct,
                                         float min_order_abs):
            TensorTradeActionScheme(portfolio, broker),
            stop(std::move(stop)),
            take(std::move(take)),
            type(tradeType),
            m_listeners(std::move(listeners)),
            min_order_abs(min_order_abs),
            min_order_pct(min_order_pct),
            m_duration(std::move(durations)),
            m_trade_sizes(std::move(trade_size))
            {}

    ManagedRiskOrders::ManagedRiskOrders(Portfolio *portfolio, Broker *broker,
                                         std::vector<float> stop,
                                         std::vector<float> take, int trade_size,
                                         std::vector<int> durations, TradeType tradeType,
                                         std::vector<OrderListener *> listeners, float min_order_pct,
                                         float min_order_abs):
    TensorTradeActionScheme(portfolio, broker),
    stop(std::move(stop)),
    take(std::move(take)),
    type(tradeType),
    m_listeners(std::move(listeners)),
    min_order_abs(min_order_abs),
    min_order_pct(min_order_pct),
    m_duration(std::move(durations)) {

        for (int i = 0; i < trade_size; ++i) {
            m_trade_sizes.push_back(float(i + 1) / trade_size );
        }


    }

    std::vector<int> ManagedRiskOrders::actionShape() {
        action_types = {this->m_portfolio->exchangePairs().size() + 1,
                        2,
                        m_trade_sizes.size(),
                        stop.size(),
                        take.size(),
                        m_duration.size()};

        for(auto a : action_types)
        {
            if(a > 1)
                d_action_shape.push_back(int(a));
        }

        return d_action_shape;
    }

    std::vector<std::shared_ptr< Order>>
    ManagedRiskOrders::getOrders(const torch::Tensor &_action, Portfolio *portfolio) {

        if(_action[1].item<int>() == 0)
            return {};

        vector<int> padded_action(6);
        padded_action[0] = _action[0].item<int>();
        padded_action[1] = _action[1].item<int>();
        padded_action[2] = _action[2].item<int>();
        padded_action[3] = action_types[3] == 0 ? -1 : action_types[3] == 1 ? 1 : _action[3].item<int>();
        padded_action[4] = action_types[4] == 0 ? -1 : action_types[4] == 1 ? 1 : _action[4].item<int>();
        padded_action[5] = action_types[5] == 0 ? -1 : action_types[5] == 1 ? 1 : _action[5].item<int>();

        auto ep = portfolio->exchangePairs()[padded_action[0]];
        auto side = padded_action[1] == 1 ? TradeSide::BUY : TradeSide::SELL;
        auto proportion = m_trade_sizes[padded_action[2]];
        float stop_ = stop[padded_action[3]];
        float take_ = take[padded_action[4]];
        auto duration = padded_action[5] == -1 ? std::nullopt :
                std::optional<int>(m_duration[padded_action[5]] + _clock.Step());

        auto instrument_ = instrument( side, ep.pair());

        auto&& wallet = portfolio->getWallet(ep.exchange()->ID(), instrument_);
        auto&& balance = wallet->Balance().asFloat();

        auto size = (balance * proportion);
        size = std::min<float>(balance, size);
        auto quantity = (size * instrument_).quantize();

        if(size < std::pow(10, -instrument_.precision) or
           size < min_order_pct * portfolio->netWorth() or
           size < min_order_abs)
        {
            return {};
        }

        auto order = riskManagedOrder(side, type, ep,
                                      ep.price(), quantity, stop_, take_, portfolio,
                                      std::nullopt, duration);

        for(auto const& listener: listeners)
            listener->onAction(padded_action);

        return {order};

    }
}
