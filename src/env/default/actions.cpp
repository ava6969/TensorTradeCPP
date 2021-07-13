//
// Created by dewe on 7/5/21.
//

#include "tensortrade/env/default/actions.h"

#include <utility>
#include "tensortrade/oms/wallets/portfolio.h"
#include "tensortrade/oms/orders/order.h"
#include "tensortrade/oms/orders/broker.h"
#include "tensortrade/oms/wallets/wallet.h"
#include "tensortrade/oms/orders/create.h"
#include "tensortrade/oms/exchanges/exchange.h"

namespace ttc
{
    TensorTradeActionScheme::TensorTradeActionScheme(Portfolio& portfolio, Broker& broker):
    m_portfolio(portfolio),
    m_broker(broker)
    {}

    void TensorTradeActionScheme::clock(Clock * clock) {
        TimeIndexed::clock(clock);

        m_portfolio.clock(clock);
        m_broker.clock(clock);
        for(auto const& exchange_ids: m_portfolio.exchanges())
        {
            auto& exchange = Exchange::exchange(exchange_ids);
            exchange.clock(clock);
        }

    }

    void TensorTradeActionScheme::perform(struct TradingEnv *env, const torch::Tensor &action) {

        auto orders = getOrders(action, m_portfolio);

        for(auto const& order : orders)
        {
            if(order) {
                //log
                m_broker.submit(order.value());
            }
        }
        m_broker.update();

    }

    Portfolio& TensorTradeActionScheme::portfolio() const {
        return m_portfolio;
    }

    void TensorTradeActionScheme::reset() {
        m_portfolio.reset();
        m_broker.reset();
    }

    std::vector<std::optional<class Order>> BSH::getOrders(torch::Tensor const& _action, Portfolio& portfolio)  {
        std::optional<Order> order;
        auto current_action = _action.item<int>();
        if(abs(current_action - action) > 0)
        {
            const auto& src = action == 0 ? cash : asset;
            const auto& tgt = action == 0 ? asset : cash;

            if(src.Balance() == 0.f)
            {
                return {};
            }

            order = proportionOrder(portfolio, src, tgt, 1.0);
            this->action = current_action;
        }

        for(auto const& listener: listeners)
            listener->onAction({action});

            return {order};
    }


    BSH::BSH(Portfolio & portfolio, Broker &  broker, const Wallet &cash, const Wallet &asset):
    TensorTradeActionScheme(portfolio, broker),cash(cash), asset(asset), action(0) {}

    SimpleOrders::SimpleOrders(Portfolio & portfolio, Broker  &  broker,
                               std::optional<std::vector<CriteriaArg>> criteria,
                               std::vector<float> trade_sizes,
                               std::optional<std::vector<int>> durations, TradeType tradeType,
                               std::optional<std::vector<OrderListener*>> listeners,
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

    SimpleOrders::SimpleOrders(Portfolio& portfolio, Broker& broker,
                               std::optional<std::vector<CriteriaArg>>  criteria,
                               int trade_size,
                               std::optional<std::vector<int>> durations,
                               TradeType tradeType,
                               std::optional<std::vector<OrderListener*>>  listeners, float min_order_pct,
                               float min_order_abs):
                                TensorTradeActionScheme(portfolio, broker),
                                m_criteria(std::move(criteria)),
                                type(tradeType),
                                m_listeners(std::move(listeners)),
                                min_order_abs(min_order_abs),
                                min_order_pct(min_order_pct),
                                m_duration(std::move(durations)){

        for (int i = 0; i < trade_size; ++i) {
            m_trade_sizes.push_back(float(i + 1) / float(trade_size) );
        }

   }


    std::vector<int> SimpleOrders::actionShape() {

        action_types = {this->m_portfolio.exchangePairs().size() + 1,
                        2,
                        m_trade_sizes.size(),
                        m_criteria.has_value() * m_criteria->size(),
                        m_duration.has_value() * m_duration->size()};

        for(auto a : action_types)
        {
            if(a > 1)
                d_action_shape.push_back(int(a));
        }

        return d_action_shape;
    }

    std::vector<std::optional<Order>> SimpleOrders::getOrders(torch::Tensor const& _action, Portfolio& portfolio)
    {
        if(_action[0].item<int>() == 0)
            return {};

        vector<int> action_index(5);
        action_index[0] = _action[0].item<int>() - 1;
        action_index[1] = _action[1].item<int>();
        action_index[2] = _action[2].item<int>();
        action_index[3] = action_types[3] == 0 ? -1 : action_types[3] == 1 ? 1 : _action[3].item<int>();
        action_index[4] = action_types[4] == 0 ? -1 : action_types[4] == 1 ? 1 : _action[4].item<int>();

        auto&& temporary_exchange_pair = portfolio.exchangePairs();
        auto&& exchange_pair = temporary_exchange_pair[action_index[0]];

        auto const& trade_side = action_index[1] == 1 ? TradeSide::BUY : TradeSide::SELL;
        auto&&      proportion = m_trade_sizes[action_index[2]];

        auto&&      trade_instrument = instrument( trade_side, exchange_pair.pair());
        auto const& exchange = Exchange::exchange(exchange_pair);
        auto const& wallet = portfolio.getWallet(exchange.ID(), trade_instrument);
        auto&&      balance = wallet.Balance().asDouble();

        auto&&      trade_size = std::min<double>(balance, balance * proportion);

        auto _criteria = action_index[3] == -1 ? std::nullopt :
                std::optional<CriteriaArg>(m_criteria->at(action_index[3]));

        auto duration = action_index[4] == -1 ? std::nullopt :
                std::optional<int>(m_duration->at(action_index[4]) + _clock->Step());

        auto quantity = (trade_size * trade_instrument).quantize();

        if(trade_size < std::pow(10, -trade_instrument.precision()) or
                trade_size < min_order_pct * portfolio.netWorth() or
                trade_size < min_order_abs)
        {
            return {};
        }

        for(auto const& listener: listeners)
            listener->onAction(action_index);

        return  {Order(_clock->Step(), trade_side, type, exchange_pair, quantity, portfolio, exchange_pair.price(), "",
                      std::nullopt, duration, _criteria)};


    }


    ManagedRiskOrders::ManagedRiskOrders(Portfolio &portfolio, Broker &broker,
                                         std::vector<float> stop,
                                         std::vector<float> take,
                                         std::vector<float> trade_size,
                                         std::optional<std::vector<int>> durations, TradeType tradeType,
                                         std::optional<std::vector<OrderListener*>> listeners,
                                         float min_order_pct, float min_order_abs):
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

    ManagedRiskOrders::ManagedRiskOrders(Portfolio& portfolio, Broker& broker,
                                         std::vector<float> stop,
                                         std::vector<float> take, int trade_size,
                                         std::optional<std::vector<int>> durations, TradeType tradeType,
                                         std::optional<std::vector<OrderListener*>> listeners,
                                         float min_order_pct, float min_order_abs):
    TensorTradeActionScheme(portfolio, broker),
    stop(std::move(stop)),
    take(std::move(take)),
    type(tradeType),
    m_listeners(std::move(listeners)),
    min_order_abs(min_order_abs),
    min_order_pct(min_order_pct),
    m_duration(std::move(durations)) {

        for (int i = 0; i < trade_size; ++i) {
            m_trade_sizes.push_back( (i + 1) / double(trade_size));
        }


    }

    std::vector<int> ManagedRiskOrders::actionShape() {

        action_types = {this->m_portfolio.exchangePairs().size() + 1, // NULL + Trade pair
                        2, // buy or sell
                        m_trade_sizes.size(),
                        stop.size(),
                        take.size(),
                        m_duration.has_value() ? m_duration->size() : 0};

        for(auto a : action_types)
        {
            if(a > 1)
                d_action_shape.push_back(int(a));
        }

        return d_action_shape;
    }

    std::vector<std::optional<Order>> ManagedRiskOrders::getOrders(const torch::Tensor &_action, Portfolio  & portfolio) {

        if(_action[0].item<int>() == 0)
            return {};

        vector<int> padded_action(6);
        padded_action[0] = _action[0].item<int>();
        padded_action[1] = _action[1].item<int>();
        padded_action[2] = _action[2].item<int>();
        padded_action[3] = action_types[3] == 0 ? -1 : action_types[3] == 1 ? 1 : _action[3].item<int>();
        padded_action[4] = action_types[4] == 0 ? -1 : action_types[4] == 1 ? 1 : _action[4].item<int>();
        padded_action[5] = action_types[5] == 0 ? -1 : action_types[5] == 1 ? 1 : _action[5].item<int>();

        auto&& ep_ = portfolio.exchangePairs();
        auto ep = ep_[padded_action[0] - 1];
        auto side = padded_action[1] == 1 ? TradeSide::BUY : TradeSide::SELL;
        auto proportion = m_trade_sizes[padded_action[2]];
        float stop_ = stop[padded_action[3]];
        float take_ = take[padded_action[4]];
        auto duration = padded_action[5] == -1 ? std::nullopt :
                std::optional<int>(m_duration->at(padded_action[5]) + _clock->Step());

        auto instrument_ = instrument( side, ep.pair());

        auto const& exchange = Exchange::exchange(ep);
        auto& wallet = portfolio.getWallet(exchange.ID(), instrument_);
        auto&& balance = wallet.Balance().asDouble();

        auto size = (balance * proportion);
        size = std::min<double>(balance, size);
        auto quantity = (size * instrument_).quantize();

        if(size < std::pow(10, -instrument_.precision()) or
           size < min_order_pct * portfolio.netWorth() or
           size < min_order_abs)
        {
            return {};
        }

        auto order = riskManagedOrder(side, type, ep,
                                      ep.price(), quantity, stop_, take_, portfolio,
                                      std::nullopt, duration);

        for(auto const& listener: listeners)
            listener->onAction(padded_action);

        return {std::move(order)};

    }
}
